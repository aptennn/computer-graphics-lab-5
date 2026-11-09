#include "lsystemwidget.h"

#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QTextStream>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace {
// Часть 1а и 1б — Михаил. Примеры записаны в формате задания по лекции 6.
const char *const presets[] = {
    "F 60 0\nF -> F-F++F-F\n",                                  // Кривая Коха
    "F+F+F+F 90 0\nF -> F+F-F-FF+F+F-F\n",                 // Квадратный остров Коха
    "X 90 0\nX -> -YF+XFX+FY-\nY -> +XF-YFY-FX+\n",       // Кривая Гильберта
    "X 90 0\nX -> X+YF+\nY -> -FX-Y\n",                    // Кривая дракона
    "F 22 90\nF -> FF-[-F+F+F]+[+F-F-F]\n",               // Куст 1
    "X 20 90\nF -> FF\nX -> F[+X]F[-X]+X\n",              // Куст 2
    "X 22.5 90\nF -> FF\nX -> F-[[X]+X]+F[+FX]-X\n"        // Куст 3 / дерево
};
constexpr int MaxCommands = 2000000;
}

// Создаёт виджет и показывает первый лекционный пример.
LSystemWidget::LSystemWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(500, 400);
    loadPreset(0);
}

// Открывает файл; при ошибке не меняет уже загруженное описание.
bool LSystemWidget::loadFile(const QString &path, QString *error)
{
    if (error) error->clear();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = QString("Не удалось открыть файл: %1").arg(file.errorString());
        return false;
    }
    QTextStream stream(&file);
    const QString source = stream.readAll();
    const QString previousAxiom = m_axiom;
    const auto previousRules = m_rules;
    const double previousAngle = m_angle;
    const double previousDirection = m_initialDirection;
    const auto previousSegments = m_segments;
    const QRectF previousBounds = m_bounds;
    const QString previousError = m_error;
    if (!parse(source, error)) return false;
    rebuild();
    if (!m_error.isEmpty()) {
        if (error) *error = m_error;
        m_axiom = previousAxiom;
        m_rules = previousRules;
        m_angle = previousAngle;
        m_initialDirection = previousDirection;
        m_segments = previousSegments;
        m_bounds = previousBounds;
        m_error = previousError;
        update();
        return false;
    }
    return true;
}

// Выбирает готовые тестовые фракталы со слайдов 31, 33, 36, 37 и 41.
void LSystemWidget::loadPreset(int index)
{
    if (index < 0 || index >= int(std::size(presets))) return;
    QString error;
    parse(QString::fromUtf8(presets[index]), &error);
    rebuild();
}

// Меняет глубину параллельной подстановки.
void LSystemWidget::setIterations(int iterations)
{
    m_iterations = std::clamp(iterations, 0, 8);
    rebuild();
}

// Меняет случайное отклонение каждого поворота.
void LSystemWidget::setAngleVariation(double degrees)
{
    m_angleVariation = std::clamp(degrees, 0.0, 45.0);
    rebuild();
}

// Повторный запуск с тем же seed создаёт тот же рисунок.
void LSystemWidget::setSeed(quint32 seed)
{
    m_seed = seed;
    rebuild();
}

// Переключает между одноцветным узором и цветным деревом.
void LSystemWidget::setTreeStyle(bool enabled)
{
    m_treeStyle = enabled;
    update();
}

// Принимает первую строку "аксиома угол направление" и строки "символ -> замена".
// Несколько правил для символа образуют равновероятные стохастические варианты.
bool LSystemWidget::parse(const QString &text, QString *error)
{
    if (error) error->clear();
    QStringList lines;
    for (QString line : text.split('\n')) {
        line = line.trimmed();
        if (!line.isEmpty() && !line.startsWith('#')) lines.append(line);
    }
    if (lines.isEmpty()) {
        if (error) *error = "Пустой файл L-системы.";
        return false;
    }
    const QStringList header = lines.first().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    bool angleOk = false, directionOk = false;
    const double angle = header.size() >= 2 ? header[1].toDouble(&angleOk) : 0.0;
    const double direction = header.size() >= 3 ? header[2].toDouble(&directionOk) : 0.0;
    if (header.size() != 3 || !angleOk || !directionOk || !std::isfinite(angle) || !std::isfinite(direction)) {
        if (error) *error = "Первая строка: <аксиома> <угол> <начальное направление>.";
        return false;
    }
    QHash<QChar, QVector<QString>> rules;
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i];
        int arrow = line.indexOf("->");
        if (arrow < 0) arrow = line.indexOf(QChar(0x2192));
        const int arrowLength = line.mid(arrow, 2) == "->" ? 2 : 1;
        if (arrow < 0) {
            if (error) *error = QString("Строка %1: ожидается правило A -> замена.").arg(i + 1);
            return false;
        }
        const QString left = line.left(arrow).trimmed();
        const QString right = line.mid(arrow + arrowLength).trimmed();
        if (left.size() != 1 || right.isEmpty() || QString("+-[]").contains(left[0])) {
            if (error) *error = QString("Строка %1: неверное правило.").arg(i + 1);
            return false;
        }
        rules[left[0]].append(right);
    }
    m_axiom = header[0];
    m_angle = angle;
    m_initialDirection = direction;
    m_rules = rules;
    return true;
}

// Каждый символ исходного поколения заменяется ровно один раз на каждой итерации.
bool LSystemWidget::expand(QString *result, QRandomGenerator &random)
{
    QString current = m_axiom;
    if (current.size() > MaxCommands) {
        m_error = "Слишком длинная строка: уменьшите размер аксиомы.";
        return false;
    }
    for (int level = 0; level < m_iterations; ++level) {
        QString next;
        for (QChar symbol : current) {
            const auto it = m_rules.constFind(symbol);
            if (it == m_rules.cend()) next += symbol;
            else {
                const auto &variants = it.value();
                next += variants.at(random.bounded(int(variants.size())));
            }
            if (next.size() > MaxCommands) {
                m_error = "Слишком длинная строка: уменьшите число итераций.";
                return false;
            }
        }
        current = std::move(next);
    }
    *result = std::move(current);
    return true;
}

// Интерпретирует F как шаг с рисованием, +/- как поворот, [/] как ветвление.
// X и Y участвуют в подстановках, но сами по себе не рисуют.
bool LSystemWidget::trace(const QString &commands, QRandomGenerator &random)
{
    struct Turtle { QPointF point; double direction; int branchDepth; int pathLength; };
    Turtle turtle{{0.0, 0.0}, std::remainder(m_initialDirection, 360.0), 0, 0};
    QVector<Turtle> stack;
    double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0;
    for (QChar command : commands) {
        if (command == 'F') {
            const double radians = qDegreesToRadians(turtle.direction);
            const QPointF next = turtle.point + QPointF(std::cos(radians), std::sin(radians));
            m_segments.append({turtle.point, next, turtle.branchDepth, turtle.pathLength});
            // QRectF::united игнорирует отрезки с нулевой шириной или высотой.
            // Экстремумы сохраняют границы и для осевых линий (например, Гильберта).
            minX = std::min(minX, next.x());
            maxX = std::max(maxX, next.x());
            minY = std::min(minY, next.y());
            maxY = std::max(maxY, next.y());
            turtle.point = next;
            ++turtle.pathLength;
        } else if (command == '+' || command == '-') {
            const double jitter = (2.0 * random.generateDouble() - 1.0) * m_angleVariation;
            const double turn = std::remainder(m_angle, 360.0) + jitter;
            turtle.direction = std::remainder(turtle.direction
                                             + (command == '+' ? turn : -turn), 360.0);
        } else if (command == '[') {
            stack.append(turtle);
            ++turtle.branchDepth;
        } else if (command == ']') {
            if (stack.isEmpty()) {
                m_error = "Лишняя закрывающая скобка в L-системе.";
                return false;
            }
            turtle = stack.takeLast();
        }
    }
    if (!stack.isEmpty()) {
        m_error = "Не закрыта ветвь в L-системе.";
        return false;
    }
    m_bounds = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
    return true;
}

// Генерирует точки после подстановок и обновляет виджет.
void LSystemWidget::rebuild()
{
    m_error.clear();
    m_segments.clear();
    QRandomGenerator random(m_seed);
    QString commands;
    if (expand(&commands, random)) trace(commands, random);
    if (!m_error.isEmpty()) m_segments.clear();
    update();
}

// Подгоняет всю геометрию к окну; для дерева меняет перо по глубине ветви.
void LSystemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(22, 28, 38));
    if (!m_error.isEmpty()) {
        painter.setPen(QColor(255, 150, 150));
        painter.drawText(rect().adjusted(20, 20, -20, -20), Qt::TextWordWrap, m_error);
        return;
    }
    if (m_segments.isEmpty()) return;
    const double margin = 26.0;
    const double rangeX = std::max(m_bounds.width(), 0.001);
    const double rangeY = std::max(m_bounds.height(), 0.001);
    const double scale = std::min((width() - 2 * margin) / rangeX,
                                  (height() - 2 * margin) / rangeY);
    const QPointF center = m_bounds.center();
    auto mapped = [&](const QPointF &p) {
        return QPointF(width() / 2.0 + (p.x() - center.x()) * scale,
                       height() / 2.0 - (p.y() - center.y()) * scale);
    };
    for (const Segment &segment : m_segments) {
        QPen pen(QColor(114, 218, 248));
        pen.setCapStyle(Qt::RoundCap);
        if (m_treeStyle) {
            const double age = segment.branchDepth * 2.0 + segment.pathLength * 0.12;
            const double green = std::clamp(age / 10.0, 0.0, 1.0);
            pen.setColor(QColor(int(111 + green * 20), int(69 + green * 100), int(38 + green * 34)));
            pen.setWidthF(std::max(1.0, 8.0 * std::pow(0.73, segment.branchDepth)
                                              * std::pow(0.975, segment.pathLength)));
        } else {
            pen.setWidthF(1.5);
        }
        painter.setPen(pen);
        painter.drawLine(mapped(segment.from), mapped(segment.to));
    }
}
