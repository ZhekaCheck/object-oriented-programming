#include <algorithm>
#include <cctype>
#include <chrono>
#include <clocale>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "GameField.h"
#include "ConcreteFactories.h"

static_assert(std::is_abstract_v<Unit>, "Unit должен быть абстрактным");
static_assert(std::has_virtual_destructor_v<Unit>, "у Unit виртуальный деструктор");
static_assert(std::is_copy_constructible_v<GameField>, "нужен копирующий конструктор");
static_assert(std::is_copy_assignable_v<GameField>, "нужно копирующее присваивание");
static_assert(std::is_nothrow_move_constructible_v<GameField>, "перемещение должно быть noexcept");
static_assert(std::is_nothrow_move_assignable_v<GameField>, "перемещение должно быть noexcept");

const int kMaxWidth = 35;
const int kMaxHeight = 40;
const int kFrameDelay = 400;

const UnitType kAllTypes[6] = {
    UnitType::Swordsman, UnitType::Spearman,
    UnitType::LightArcher, UnitType::HeavyArcher,
    UnitType::Cataphract, UnitType::HorseArcher
};

const UnitFactory& factoryFor(UnitType type) {
    static InfantryFactory infantry;
    static ArcherFactory archers;
    static CavalryFactory cavalry;
    switch (type) {
    case UnitType::Swordsman:
    case UnitType::Spearman: return infantry;
    case UnitType::LightArcher:
    case UnitType::HeavyArcher: return archers;
    default: return cavalry;
    }
}

UnitType randomType() { return kAllTypes[std::rand() % 6]; }

char symbolOf(UnitType type) {
    switch (type) {
    case UnitType::Swordsman: return 'M';
    case UnitType::Spearman: return 'K';
    case UnitType::LightArcher: return 'L';
    case UnitType::HeavyArcher: return 'T';
    case UnitType::Cataphract: return 'C';
    case UnitType::HorseArcher: return 'H';
    }
    return '?';
}

char unitSymbol(const Unit& u) {
    char c = symbolOf(u.getType());
    return u.isAlive() ? c : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

std::vector<std::string> renderField(const GameField& field) {
    std::vector<std::string> lines;

    std::string border;
    for (int i = 0; i < field.width() + 2; ++i) {
        border += 'X';
        if (i < field.width() + 1) border += ' ';
    }

    lines.push_back(border);
    for (int y = 0; y < field.height(); ++y) {
        std::string row = "X ";
        for (int x = 0; x < field.width(); ++x) {
            const Unit* u = field.getUnit(x, y);
            row += u ? unitSymbol(*u) : ' ';
            row += ' ';
        }
        row += 'X';
        lines.push_back(row);
    }
    lines.push_back(border);
    return lines;
}

std::vector<std::string> menuLines() {
    return {
        "ОСНОВНОЙ ФУНКЦИОНАЛ:",
        "1. Добавить юнита (фабрика)",
        "2. Удалить юнита (x, y)",
        "3. Удалить юнита (индекс ячейки)",
        "4. Переместить юнита",
        "5. Информация о юните",
        "6. Нанести урон юниту",
        "7. Атака: юнит бьёт юнита",
        "8. Массовый урон всем (итератор)",
        "9. Убрать погибших (итератор)",
        "10. Список юнитов (итератор)",
        "ПРОЧИЙ ФУНКЦИОНАЛ:",
        "11. Случайная армия (фабрики)",
        "12. Очистить поле",
        "13. Сценарий 1: ядерный удар",
        "14. Сценарий 2: местная междоусобица",
        "15. Сценарий 3: типовая мировая война",
        "16. Создать новое поле",
        "0. Выход"
    };
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void drawScreen(const GameField& field, const std::string& message) {
    clearScreen();

    std::vector<std::string> left = renderField(field);
    std::vector<std::string> right = menuLines();
    std::size_t leftWidth = left.empty() ? 0 : left[0].size();
    std::size_t rows = std::max(left.size(), right.size());

    for (std::size_t i = 0; i < rows; ++i) {
        std::string l = i < left.size() ? left[i] : std::string();
        l.resize(leftWidth, ' ');
        std::cout << l << "    " << (i < right.size() ? right[i] : std::string()) << "\n";
    }

    std::cout << "Поле " << field.width() << "x" << field.height() << ", юнитов: " << field.size() << "/" << field.maxUnits() << "\n";
    if (!message.empty()) std::cout << ">> " << message << "\n";
}

void showFrame(const GameField& field, const std::string& title) {
    clearScreen();
    std::cout << title << "\n\n";
    for (const std::string& line : renderField(field)) std::cout << line << "\n";
    std::cout << "\nЮнитов: " << field.size() << "/" << field.maxUnits() << "\n";
    std::cout << "\nНажмите Enter, чтобы продолжить...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

int readInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            return value;
        }
        if (std::cin.eof()) std::exit(0);
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Нужно ввести число.\n";
    }
}

int readIntInRange(const std::string& prompt, int lo, int hi) {
    while (true) {
        int v = readInt(prompt);
        if (v >= lo && v <= hi) return v;
        std::cout << "Введите число от " << lo << " до " << hi << ".\n";
    }
}

void waitEnter() {
    std::cout << "\nНажмите Enter, чтобы продолжить...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::string addFailureReason(const GameField& f, int x, int y) {
    if (x < 0 || y < 0 || x >= f.width() || y >= f.height()) return "координаты вне поля";
    if (f.getUnit(x, y)) return "клетка занята";
    if (f.size() >= f.maxUnits()) return "достигнут лимит юнитов";
    return "неизвестная причина";
}

std::string actAdd(GameField& field) {
    std::cout << "\nВиды юнитов:\n";
    for (int i = 0; i < 6; ++i)
        std::cout << "  " << i + 1 << " - " << toString(kAllTypes[i])
        << " [" << symbolOf(kAllTypes[i]) << "]\n";

    int k = readInt("Вид (1-6): ");
    if (k < 1 || k > 6) return "Неверный номер вида юнита.";
    int x = readInt("x: ");
    int y = readInt("y: ");

    UnitType t = kAllTypes[k - 1];
    if (field.addUnit(factoryFor(t), t, x, y))
        return std::string("Добавлен юнит ") + toString(t) + " в (" +
        std::to_string(x) + "," + std::to_string(y) + ").";
    return "Не добавлено: " + addFailureReason(field, x, y) + ".";
}

std::string actRemoveXY(GameField& field) {
    int x = readInt("x: ");
    int y = readInt("y: ");
    return field.removeUnit(x, y) ? "Юнит удалён." : "Там нет юнита (или координаты вне поля).";
}

std::string actRemoveIndex(GameField& field) {
    std::cout << "Индекс ячейки = y * ширина + x (от 0 до "
        << field.width() * field.height() - 1 << ")\n";
    int idx = readInt("Индекс: ");
    if (idx < 0) return "Индекс не может быть отрицательным.";
    return field.removeByIndex(static_cast<std::size_t>(idx))
        ? "Юнит удалён." : "В этой ячейке нет юнита (или индекс вне поля).";
}

std::string actMove(GameField& field) {
    int x1 = readInt("Откуда x: ");
    int y1 = readInt("Откуда y: ");
    int x2 = readInt("Куда x: ");
    int y2 = readInt("Куда y: ");
    return field.moveUnit(x1, y1, x2, y2)
        ? "Юнит перемещён."
        : "Не перемещён: нет юнита в начальной клетке, цель занята или вне поля.";
}

std::string actInfo(const GameField& field) {
    int x = readInt("x: ");
    int y = readInt("y: ");
    const Unit* u = field.getUnit(x, y);
    if (!u) return "Там нет юнита.";

    std::cout << "\nВид: " << toString(u->getType()) << " [" << unitSymbol(*u) << "]\n"
        << "Позиция: (" << u->x() << "," << u->y() << ")\n"
        << "Здоровье: " << u->health().value() << "\n"
        << "Броня: " << u->armor().value() << "\n"
        << "Атака: " << u->attack().value() << "\n";
    if (const Archer* a = dynamic_cast<const Archer*>(u))
        std::cout << "Дальность: " << a->range() << "\n";
    if (const Cavalry* c = dynamic_cast<const Cavalry*>(u))
        std::cout << "Скорость: " << c->speed() << "\n";
    waitEnter();
    return "Информация показана.";
}

std::string actDamage(GameField& field) {
    int x = readInt("x: ");
    int y = readInt("y: ");
    int amount = readInt("Урон (до учёта брони): ");
    Unit* u = field.getUnit(x, y);
    if (!u) return "Там нет юнита.";
    if (amount < 0) return "Урон не может быть отрицательным.";
    int before = u->health().value();
    u->takeDamage(amount);
    return std::string(toString(u->getType())) + ": HP " + std::to_string(before) +
        " -> " + std::to_string(u->health().value()) + ".";
}

std::string actAttack(GameField& field) {
    int x1 = readInt("Атакующий x: ");
    int y1 = readInt("Атакующий y: ");
    int x2 = readInt("Цель x: ");
    int y2 = readInt("Цель y: ");
    Unit* attacker = field.getUnit(x1, y1);
    Unit* target = field.getUnit(x2, y2);
    if (!attacker || !target) return "В одной из клеток нет юнита.";
    if (attacker == target) return "Юнит не может атаковать сам себя.";
    if (!attacker->isAlive()) return "Погибший юнит не может атаковать.";

    int before = target->health().value();
    int dmg = attacker->calcDamageTo(*target);
    target->takeDamage(attacker->attack().value());
    return std::string(toString(attacker->getType())) + " бьёт " + toString(target->getType()) +
        ": урон " + std::to_string(dmg) + ", HP " + std::to_string(before) +
        " -> " + std::to_string(target->health().value()) + ".";
}

std::string actMassDamage(GameField& field) {
    int amount = readInt("Урон каждому юниту: ");
    if (amount < 0) return "Урон не может быть отрицательным.";
    int count = 0;
    for (Unit& u : field) {
        u.takeDamage(amount);
        ++count;
    }
    return "Урон " + std::to_string(amount) + " нанесён юнитам: " + std::to_string(count) + ".";
}

int removeDead(GameField& field) {
    int removed = 0;
    for (auto it = field.begin(); it != field.end(); ++it) {
        if (!it->isAlive()) {
            field.removeUnit(it->x(), it->y());
            ++removed;
        }
    }
    return removed;
}

std::string actRemoveDead(GameField& field) {
    return "Убрано погибших: " + std::to_string(removeDead(field)) + ".";
}

std::string actList(const GameField& field) {
    std::cout << "\nВсе юниты на поле (" << field.size() << "):\n";
    int n = 0;
    for (const Unit& u : field) {
        std::cout << "  " << ++n << ".[" << unitSymbol(u) << "]" << toString(u.getType())
            << "(" << u.x() << "," << u.y() << ")"
            << "\nHealth Point = " << u.health().value()
            << "\nArmor = " << u.armor().value()
            << "\nAttack = " << u.attack().value() << "\n";
    }
    if (n == 0) std::cout << "(пусто)\n";
    waitEnter();
    return "Список показан.";
}

std::string actRandomArmy(GameField& field) {
    int requested = readInt("Сколько юнитов добавить: ");
    if (requested <= 0) return "Нужно положительное число.";

    const std::size_t before = field.size();

    std::vector<std::pair<int, int>> freeCells;
    for (int y = 0; y < field.height(); ++y)
        for (int x = 0; x < field.width(); ++x)
            if (!field.getUnit(x, y)) freeCells.push_back(std::make_pair(x, y));

    std::size_t room = field.maxUnits() > before ? field.maxUnits() - before : 0;
    std::size_t capacity = std::min(room, freeCells.size());

    for (std::size_t i = freeCells.size(); i > 1; --i)
        std::swap(freeCells[i - 1], freeCells[static_cast<std::size_t>(std::rand()) % i]);

    std::size_t toAdd = std::min(static_cast<std::size_t>(requested), capacity);
    for (std::size_t i = 0; i < toAdd; ++i) {
        UnitType t = randomType();
        field.addUnit(factoryFor(t), t, freeCells[i].first, freeCells[i].second);
    }

    std::string msg = "Добавлено юнитов: " + std::to_string(toAdd) +
        " из " + std::to_string(requested) + ".";
    if (toAdd < static_cast<std::size_t>(requested)) {
        if (room <= freeCells.size())
            msg += " Достигнут лимит " + std::to_string(field.maxUnits()) +
            " (до этого на поле уже было " + std::to_string(before) + ").";
        else
            msg += " Свободные клетки закончились.";
    }
    return msg;
}

std::string actNewField(GameField& field) {
    int w = readIntInRange("Ширина поля (1-" + std::to_string(kMaxWidth) + "): ", 1, kMaxWidth);
    int h = readIntInRange("Высота поля (1-" + std::to_string(kMaxHeight) + "): ", 1, kMaxHeight);
    int m = readIntInRange("Максимум юнитов (1-" + std::to_string(w * h) + "): ", 1, w * h);
    field = GameField(w, h, static_cast<std::size_t>(m));
    return "Создано новое поле " + std::to_string(w) + "x" + std::to_string(h) +
        ", лимит юнитов " + std::to_string(m) + ".";
}

int blockRows(int count, int height, int maxCols) {
    int rows = 1;
    while (rows < height && (rows * rows < count || (count + rows - 1) / rows > maxCols)) ++rows;
    return rows;
}

int placeBlock(GameField& field, const std::vector<UnitType>& kinds, int count, int rows, int startX, int startY, std::vector<Unit*>& army) {
    int placed = 0;
    for (int x = startX; x < field.width() && placed < count; ++x) {
        for (int y = startY; y < startY + rows && placed < count; ++y) {
            UnitType t = kinds[std::rand() % kinds.size()];
            if (field.addUnit(factoryFor(t), t, x, y)) {
                army.push_back(field.getUnit(x, y));
                ++placed;
            }
        }
    }
    return placed;
}

int buildTwoArmies(GameField& field, std::vector<Unit*>& left, std::vector<Unit*>& right) {
    const int gap = 2;
    int maxCols = (field.width() - gap) / 2;
    int count = std::min(static_cast<int>(field.maxUnits() / 2), maxCols * field.height());
    if (count <= 0) return 0;

    int rows = blockRows(count, field.height(), maxCols);
    count = count / rows * rows;
    int cols = count / rows;
    int startX = (field.width() - (2 * cols + gap)) / 2;
    int startY = (field.height() - rows) / 2;

    std::vector<UnitType> leftKinds = { UnitType::Cataphract };
    std::vector<UnitType> rightKinds = { UnitType::Swordsman, UnitType::Spearman, UnitType::LightArcher, UnitType::HeavyArcher };
    placeBlock(field, leftKinds, count, rows, startX, startY, left);
    placeBlock(field, rightKinds, count, rows, startX + cols + gap, startY, right);
    return count;
}

int distanceBetween(const Unit& a, const Unit& b) {
    return std::max(std::abs(a.x() - b.x()), std::abs(a.y() - b.y()));
}

int countAlive(const std::vector<Unit*>& army) {
    int alive = 0;
    for (Unit* u : army)
        if (u->isAlive()) ++alive;
    return alive;
}

Unit* findNearestEnemy(const std::vector<std::vector<Unit*>>& armies, int own, const Unit& unit) {
    Unit* nearest = nullptr;
    int best = 1000000;
    for (int i = 0; i < static_cast<int>(armies.size()); ++i) {
        if (i == own) continue;
        for (Unit* e : armies[i]) {
            if (!e->isAlive()) continue;
            int d = distanceBetween(unit, *e);
            if (d < best) {
                best = d;
                nearest = e;
            }
        }
    }
    return nearest;
}

void stepToward(GameField& field, const Unit& unit, const Unit& target) {
    int dx = (target.x() > unit.x()) - (target.x() < unit.x());
    int dy = (target.y() > unit.y()) - (target.y() < unit.y());
    int x = unit.x();
    int y = unit.y();
    if (field.moveUnit(x, y, x + dx, y + dy)) return;
    if (dx != 0 && field.moveUnit(x, y, x + dx, y)) return;
    if (dy != 0 && field.moveUnit(x, y, x, y + dy)) return;
}

bool othersAlive(const std::vector<std::vector<Unit*>>& armies, int winner) {
    for (int i = 0; i < static_cast<int>(armies.size()); ++i)
        if (i != winner && !armies[i].empty()) return true;
    return false;
}

void removeCasualties(GameField& field, std::vector<std::vector<Unit*>>& armies) {
    for (std::vector<Unit*>& army : armies)
        army.erase(std::remove_if(army.begin(), army.end(), [](Unit* u) { return !u->isAlive(); }), army.end());
    removeDead(field);
}

void showBattleFrame(const GameField& field, const std::string& title) {
    clearScreen();
    std::cout << title << "\n\n";
    for (const std::string& line : renderField(field)) std::cout << line << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelay));
}

int battle(GameField& field, std::vector<std::vector<Unit*>>& armies, const std::vector<std::string>& names, int winner) {
    int round = 0;
    bool winnerLost = false;
    while (othersAlive(armies, winner) && round < 300) {
        ++round;

        for (int i = 0; i < static_cast<int>(armies.size()); ++i) {
            for (Unit* u : armies[i]) {
                Unit* enemy = findNearestEnemy(armies, i, *u);
                if (enemy && distanceBetween(*u, *enemy) > 1) stepToward(field, *u, *enemy);
            }
        }

        for (int i = 0; i < static_cast<int>(armies.size()); ++i) {
            for (Unit* u : armies[i]) {
                if (!u->isAlive()) continue;
                Unit* enemy = findNearestEnemy(armies, i, *u);
                if (!enemy || distanceBetween(*u, *enemy) > 1) continue;
                if (i == winner) {
                    if (countAlive(armies[i]) > 1 && (!winnerLost || std::rand() % 100 < 8)) {
                        u->takeDamage(1000000);
                        winnerLost = true;
                    }
                }
                else if (std::rand() % 100 < 30) {
                    u->takeDamage(1000000);
                }
            }
        }

        std::string title = "Раунд " + std::to_string(round) + ":";
        for (int i = 0; i < static_cast<int>(armies.size()); ++i)
            title += " " + names[i] + " - " + std::to_string(countAlive(armies[i])) + ";";
        showBattleFrame(field, title);
        removeCasualties(field, armies);
    }
    return round;
}

std::string scenarioNuke(GameField& field) {
    field.clear();
    drawScreen(field, "Сценарий 1: поле очищено. Сейчас две армии встанут друг напротив друга.");
    waitEnter();

    std::vector<Unit*> left;
    std::vector<Unit*> right;
    int count = buildTwoArmies(field, left, right);
    if (count == 0)
        return "Сценарий 1: поле слишком маленькое (нужна ширина от 4 и лимит от 2 юнитов).";

    drawScreen(field, "Шаг 1: две армии по " + std::to_string(count) + " юнитов стоят друг напротив друга.");
    waitEnter();

    for (Unit& u : field) u.takeDamage(1000000);
    showFrame(field, "Шаг 2: ядерный удар. На поле остались одни трупы.");

    return "Сценарий 1 завершён: обе армии уничтожены одним ударом.";
}

std::string scenarioCataphractsWin(GameField& field) {
    field.clear();
    drawScreen(field, "Сценарий 2: поле очищено. Сейчас две армии встанут друг напротив друга.");
    waitEnter();

    std::vector<std::vector<Unit*>> armies(2);
    int count = buildTwoArmies(field, armies[0], armies[1]);
    if (count == 0)
        return "Сценарий 2: поле слишком маленькое (нужна ширина от 4 и лимит от 2 юнитов).";

    drawScreen(field, "Шаг 1: катафракты (слева) и пехота с лучниками (справа), по " + std::to_string(count) + " юнитов. Дальше бой пойдёт сам.");
    waitEnter();

    std::vector<std::string> names = { "Катафракты", "Пехота и лучники" };
    int rounds = battle(field, armies, names, 0);

    drawScreen(field, "Катафракты победили: выжило " + std::to_string(armies[0].size()) + " из " +
        std::to_string(count) + ", противников не осталось.");
    waitEnter();
    return "Сценарий 2 завершён: катафракты победили за " + std::to_string(rounds) + " раундов.";
}

std::string scenarioWorldWarByType(GameField& field) {
    field.clear();
    drawScreen(field, "Сценарий 3: поле очищено. Сейчас три армии встанут по углам поля.");
    waitEnter();

    int maxCols = field.width() / 3;
    int maxRows = field.height() / 3;
    int count = std::min(static_cast<int>(field.maxUnits() / 3), maxCols * maxRows);
    if (count <= 0)
        return "Сценарий 3: поле слишком маленькое (нужна ширина и высота от 3 и лимит от 3 юнитов).";

    int rows = blockRows(count, maxRows, maxCols);
    count = count / rows * rows;
    int cols = count / rows;

    UnitType kinds[3] = { UnitType::Swordsman, UnitType::HeavyArcher, UnitType::Cataphract };
    std::vector<std::string> names = { "Пехота", "Лучники", "Конница" };
    std::vector<std::vector<Unit*>> armies(3);

    int startX[3] = { 0, field.width() - cols, 0 };
    int startY[3] = { 0, 0, field.height() - rows };
    for (int i = 0; i < 3; ++i)
        placeBlock(field, { kinds[i] }, count, rows, startX[i], startY[i], armies[i]);

    drawScreen(field, "Шаг 1: три армии по " + std::to_string(count) +
        " юнитов в углах поля: пехота, лучники и конница. Дальше война пойдёт сама.");
    waitEnter();

    int winner = std::rand() % 3;
    int rounds = battle(field, armies, names, winner);

    drawScreen(field, "Победа: " + names[winner] + ". Выжило " + std::to_string(armies[winner].size()) +
        " из " + std::to_string(count) + ".");
    waitEnter();
    return "Сценарий 3 завершён: победила армия \"" + names[winner] + "\" за " + std::to_string(rounds) + " раундов.";
}

int main() {
    setlocale(LC_ALL, "rus");
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::cout << "Установите размеры игрового поля:\n";
    int w = readIntInRange("Ширина поля (1-" + std::to_string(kMaxWidth) + "): ", 1, kMaxWidth);
    int h = readIntInRange("Высота поля (1-" + std::to_string(kMaxHeight) + "): ", 1, kMaxHeight);
    int m = readIntInRange("Максимум юнитов (1-" + std::to_string(w * h) + "): ", 1, w * h);

    GameField field(w, h, static_cast<std::size_t>(m));

    std::string message = "Добро пожаловат, Артем Геннадьевич. Приятной игры!";

    while (true) {
        drawScreen(field, message);
        int choice = readInt("\nВыбор: ");
        message.clear();

        try {
            switch (choice) {
            case 0: return 0;
            case 1: message = actAdd(field); break;
            case 2: message = actRemoveXY(field); break;
            case 3: message = actRemoveIndex(field); break;
            case 4: message = actMove(field); break;
            case 5: message = actInfo(field); break;
            case 6: message = actDamage(field); break;
            case 7: message = actAttack(field); break;
            case 8: message = actMassDamage(field); break;
            case 9: message = actRemoveDead(field); break;
            case 10: message = actList(field); break;
            case 11: message = actRandomArmy(field); break;
            case 12: field.clear(); message = "Поле очищено."; break;
            case 13: message = scenarioNuke(field); break;
            case 14: message = scenarioCataphractsWin(field); break;
            case 15: message = scenarioWorldWarByType(field); break;
            case 16: message = actNewField(field); break;
            default: message = "Нет такого пункта меню.";
            }
        }
        catch (const std::exception& e) {
            message = std::string("Ошибка: ") + e.what();
        }
    }
}