# Морской бой

На стандартном поле 10×10 с флотом из десяти кораблей доступны два компьютерных соперника. `Easy` загружает `models/easy.txt` и играет исходной полносвязной сетью. `Hard` загружает `models/hard.txt` и использует одну общую сеть для оценки всех клеток. Расчёт выполняется на CPU. Предыдущие версии весов и результаты экспериментов хранятся в папке `old_models/`.

## Выстрел в игре

Создай соперника один раз в начале партии. Второй аргумент задаёт порядок выбора между клетками с одинаковой оценкой. По умолчанию программа ищет папку `models` в рабочем каталоге. Путь к ней можно передать третьим аргументом.

```cpp
#include "Game/computer_opponent.hpp"

ComputerOpponent opponent(ComputerOpponent::Difficulty::Hard, 42);
auto [x, y] = opponent.ChooseShot(board);
board.Shoot(x, y);
```

## Где писать игру

Точка входа — `main.cpp`. Сейчас там заглушка для будущего интерфейса. Доска и правила выстрела находятся в `Game/board.hpp`, оба уровня сложности доступны через `Game/computer_opponent.hpp`.

`HardNetwork` в `neural_network/hard_network.hpp` — реализация соперника Hard. Она загружает его веса, оценивает каждую необстрелянную клетку и выбирает выстрел. Оба тренера и их вспомогательные файлы находятся в `neural_network/training/`. Обновление и сохранение весов Hard реализованы в `neural_network/training/hard_network_trainer.cpp`. Из интерфейса достаточно вызывать `ComputerOpponent::ChooseShot`.

## Сборка

Обычная сборка VS Code собирает `app` с игровым `main.cpp`. Для полносвязной сети предусмотрен режим CMake `TRAIN_NETWORK=ON`. Тренер Hard собирается отдельно по явному запросу:

```powershell
cmake --build --preset local-release --target local_train
```
