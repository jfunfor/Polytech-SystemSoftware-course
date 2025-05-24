# Тема Git. Задание 1. Управление репозиторием.

Вам необходимо создать структурированный репозиторий для управления решениями задач, внести изменения в код и выполнить слияния веток с соблюдением правил Conventional Commits.

## Условие задания

1. Инициализируйте репозиторий в каталоге `~/random_tasks/`.
2. В ветке `main` создайте три коммита:
   - Каждый коммит должен содержать **один файл-решение** (например, `task1.sh`, `task2.sh`, `task3.sh`).
3. Создайте ветку `feature/another_task`:
   - Добавьте файл `task4.sh` с решением четвертой задачи.
   - Внесите изменения в `task4.sh` и зафиксируйте их отдельным коммитом.
4. Выполните слияние ветки `feature/another_task` в `main`.
5. Создайте ветку `fix/task1`:
   - Исправьте решение задачи 1.
   - Слейте ветку в `main`.
6. Создайте ветку `fix/task2`:
   - Удалите файл-решение задачи 2.
   - Слейте ветку в `main`.

## Советы

- Для работы с ветками используйте:
  ```bash
  git checkout -b <branch_name>  # Создание и переключение на ветку
  git merge <branch_name>        # Слияние ветки
  ```
- Удаление файлов через git:
    ```
    git rm <filename>              # Удаление файла с фиксацией
    ```
- Все коммиты должны соответствовать Conventional Commits. Примеры:
    ```
    feat: add task 1 solution
    fix: correct syntax in task4.sh
    fix: remove task2 solution.
    ```

## Автоматическая проверка

1. Проверка наличия трех исходных файлов-решений в истории main.
2. Проверка корректности слияния ветки feature/another_task (наличие task4.sh и его изменений).
3. Проверка отсутствия task2.sh в main после слияния fix/task2.
4. Проверка формата коммитов (наличие префиксов feat, fix и описаний).

## Скрипт автопроверки

```bash
#!/bin/bash

# Проверка задания Git 1

REPO_DIR="$HOME/random_tasks"
ERRORS=0

# Проверка существования репозитория
check_repo() {
    if [ ! -d "$REPO_DIR/.git" ]; then
        echo "❌ Репозиторий не инициализирован в $REPO_DIR"
        exit 1
    fi
}

# Проверка коммитов в main
check_initial_commits() {
    echo "🔍 Проверка начальных коммитов..."
    cd "$REPO_DIR" || exit 1
    
    local commits=$(git log --oneline main | grep -E "feat: add (task1|task2|task3)\.sh" | wc -l)
    if [ "$commits" -ne 3 ]; then
        echo "❌ Не найдено 3 корректных коммитов в main"
        ((ERRORS++))
    fi
}

# Проверка task4.sh
check_task4() {
    echo "🔍 Проверка task4.sh..."
    if [ ! -f "$REPO_DIR/task4.sh" ]; then
        echo "❌ Файл task4.sh не найден"
        ((ERRORS++))
    else
        local changes=$(git log --oneline main | grep -E "fix: update task4 script")
        if [ -z "$changes" ]; then
            echo "❌ Изменения task4.sh не зафиксированы"
            ((ERRORS++))
        fi
    fi
}

# Проверка удаления task2.sh
check_task2_removal() {
    echo "🔍 Проверка удаления task2.sh..."
    if [ -f "$REPO_DIR/task2.sh" ]; then
        echo "❌ Файл task2.sh до сих пор существует"
        ((ERRORS++))
    fi
    
    local removal_commit=$(git log --oneline main | grep "fix: remove task2 solution")
    if [ -z "$removal_commit" ]; then
        echo "❌ Коммит удаления task2.sh не найден"
        ((ERRORS++))
    fi
}

# Проверка формата коммитов
check_commit_format() {
    echo "🔍 Проверка формата коммитов..."
    git log main --oneline | grep -v -E "(^Merge|^feat:|^fix:)" | while read -r line; do
        if [ -n "$line" ]; then
            echo "❌ Нарушен формат коммита: $line"
            ((ERRORS++))
        fi
    done
}

# Главная функция
main() {
    check_repo
    check_initial_commits
    check_task4
    check_task2_removal
    check_commit_format

    if [ "$ERRORS" -eq 0 ]; then
        echo "✅ Все проверки пройдены успешно!"
    else
        echo "❌ Найдено ошибок: $ERRORS"
        exit 1
    fi
}

main
```
