# Тема Git. Задание 3. Работа с историей и конфликтами.

Вам необходимо освоить продвинутые техники управления историей: `cherry-pick`, `revert`, а также научиться разрешать конфликты слияния.

## Условие задания

1. **Создайте репозиторий** в каталоге `~/history_lab/`:
   - Добавьте файл `calculator.py` с базовой функцией сложения. Закоммитьте с сообщением `feat: add sum function`.
   - Добавьте функцию вычитания в тот же файл. Закоммитьте с сообщением `feat: add subtract function`.
   - Создайте ветку `hotfix`, исправьте ошибку в функции сложения (например, обработку отрицательных чисел). Закоммитьте с сообщением `fix: correct sum logic`.

2. **Используйте `revert`**:
   - В ветке `main` отмените коммит с добавлением функции вычитания (`revert`).

3. **Примените `cherry-pick`**:
   - Перенесите коммит с исправлением из ветки `hotfix` в `main` с помощью `cherry-pick`.

4. **Создайте конфликт и разрешите его**:
   - В ветке `main` добавьте функцию умножения. Закоммитьте с сообщением `feat: add multiply`.
   - В ветке `hotfix` добавьте функцию деления в тот же файл. Закоммитьте с сообщением `feat: add divide`.
   - Выполните слияние `hotfix` в `main`, разрешите конфликт вручную и зафиксируйте результат.

## Советы

- Для `cherry-pick`:
  ```bash
  git cherry-pick <commit-hash>  # Перенос конкретного коммита
  ```
- Для `revert`:
    ```
    git revert <commit-hash>       # Отмена изменений коммита
    ```
- При конфликтах:
Используйте git status для поиска конфликтующих файлов. Отредактируйте файлы, затем выполните:
    ```
    git add <file>
    git commit
    ```

## Автоматическая проверка

1. Проверка наличия коммита revert в истории main.
2. Проверка применения коммита из hotfix через cherry-pick.
3. Проверка успешного слияния hotfix в main с разрешенным конфликтом.
4. Наличие всех функций (sum, subtract, multiply, divide) в итоговом calculator.py.

## Скрипт автопроверки

```bash
#!/bin/bash

REPO_DIR="$HOME/history_lab"
ERRORS=0

check_repo() {
    if [ ! -d "$REPO_DIR/.git" ]; then
        echo "❌ Репозиторий не найден в $REPO_DIR"
        exit 1
    fi
}

check_revert() {
    echo "🔍 Проверка revert..."
    if ! git -C "$REPO_DIR" log main --oneline | grep -q "revert: feat: add subtract function"; then
        echo "❌ Коммит revert не найден"
        ((ERRORS++))
    fi
}

check_cherrypick() {
    echo "🔍 Проверка cherry-pick..."
    local hotfix_commit=$(git -C "$REPO_DIR" log hotfix --format=%H -n 1)
    if ! git -C "$REPO_DIR" log main --format=%H | grep -q "$hotfix_commit"; then
        echo "❌ Исправление из hotfix не перенесено в main"
        ((ERRORS++))
    fi
}

check_merge() {
    echo "🔍 Проверка слияния..."
    if ! git -C "$REPO_DIR" branch --merged main | grep -q "hotfix"; then
        echo "❌ Ветка hotfix не слита в main"
        ((ERRORS++))
    fi

    if [ ! -f "$REPO_DIR/calculator.py" ] || ! grep -q "divide" "$REPO_DIR/calculator.py"; then
        echo "❌ Функция divide отсутствует"
        ((ERRORS++))
    fi
}

main() {
    check_repo
    check_revert
    check_cherrypick
    check_merge

    if [ "$ERRORS" -eq 0 ]; then
        echo "✅ Все проверки пройдены!"
    else
        echo "❌ Найдено ошибок: $ERRORS"
        exit 1
    fi
}

main
```
