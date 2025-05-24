# Тема Git. Задание 2. Работа с удаленным репозиторием.

Вам необходимо создать локальный и удаленный репозиторий, организовать совместную работу через GitHub, выполнить слияние изменений.

## Условие задания

1. **Создайте локальный репозиторий** в каталоге `~/online_repo/`:
   - Добавьте три файла: `task1.sh`, `task2.sh`, `task3.sh`.
   - Закоммитьте каждый файл **отдельным коммитом** с сообщениями в формате Conventional Commits (например, `feat: add task1`).

2. **Создайте удаленный репозиторий** на GitHub (или другом сервисе):
   - Добавьте remote с именем `online`, указывающий на ваш удаленный репозиторий.
   - Загрузите ветку `main` на remote:  
     ```bash
     git push -u online main
     ```

3. **Совместная работа**:
   - Попросите товарища склонировать ваш репозиторий.
   - В его локальной копии:
     - Создайте ветку `feature/name_tasks` (замените `name` на имя участника).
     - Добавьте три файла с задачами в каталог `examples/` (например, `examples/task_a.py`, `examples/task_b.py`).
     - Сделайте **три отдельных коммита**.
     - Отправьте ветку на удаленный репозиторий:  
       ```bash
       git push -u online feature/name_tasks
       ```

4. **Внесите изменения и выполните слияние**:
   - В вашем локальном репозитории:
     - Переключитесь на ветку `feature/name_tasks`.
     - Измените любой файл в каталоге `examples/`, закоммитьте изменения.
     - Выполните слияние ветки `feature/name_tasks` в `main` и отправьте результат на remote.

## Советы

- Для работы с удаленным репозиторием:
  ```bash
  git remote add online <URL>    # Добавить remote
  git fetch online              # Получить изменения
  git merge --no-ff             # Слияние с сохранением истории

## Автоматическая проверка
1. Проверка наличия трех исходных коммитов в main с префиксом feat:.
2. Проверка существования ветки feature/name_tasks на удаленном репозитории.
3. Проверка наличия каталога examples/ с файлами после слияния.
4. Проверка корректности формата коммитов (Conventional Commits).

## Скрипт автопроверки

```bash
#!/bin/bash

REPO_DIR="$HOME/online_repo"
REMOTE_URL=$(git -C "$REPO_DIR" remote get-url online 2>/dev/null)
ERRORS=0

check_local_repo() {
    if [ ! -d "$REPO_DIR/.git" ]; then
        echo "❌ Локальный репозиторий не найден в $REPO_DIR"
        exit 1
    fi
}

check_remote_connection() {
    if [ -z "$REMOTE_URL" ]; then
        echo "❌ Remote 'online' не настроен"
        ((ERRORS++))
    fi
}

check_initial_commits() {
    echo "🔍 Проверка коммитов в main..."
    local commits=$(git -C "$REPO_DIR" log main --oneline | grep -c "feat: add task")
    if [ "$commits" -lt 3 ]; then
        echo "❌ В main меньше 3 коммитов с задачами"
        ((ERRORS++))
    fi
}

check_feature_branch() {
    echo "🔍 Проверка ветки feature/name_tasks..."
    if ! git -C "$REPO_DIR" ls-remote --exit-code --heads online "feature/*_tasks" >/dev/null; then
        echo "❌ Ветка feature/name_tasks не найдена на remote"
        ((ERRORS++))
    fi
}

main() {
    check_local_repo
    check_remote_connection
    check_initial_commits
    check_feature_branch

    if [ "$ERRORS" -eq 0 ]; then
        echo "✅ Все проверки пройдены!"
    else
        echo "❌ Найдено ошибок: $ERRORS"
        exit 1
    fi
}

main
```