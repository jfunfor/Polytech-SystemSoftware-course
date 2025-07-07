## **Git Hooks: Автоматизируем рутину**  


### **1. Что такое Git Hooks?**  

**Гит хуки** — это скрипты, которые Git запускает автоматически при определённых событиях (например, перед коммитом или после пуша).  

 **Зачем нужны?**  
- Проверить код перед коммитом (нет ли debug-выводов?)  
- Запустить тесты  
- Автоматически форматировать код  
- Запретить пушить в `main`  
- Уведомить команду в Slack о новом коммите  

**Это как "ассистенты", которые следят за порядком в вашем репозитории.**  

---  

### **2. Какие бывают хуки?**  

#### **Клиентские (локальные)**  
Срабатывают на вашей машине:  
- `pre-commit` — перед созданием коммита *(чаще всего используется!)*  
- `prepare-commit-msg` — перед открытием редактора сообщения  
- `commit-msg` — после ввода сообщения  
- `pre-push` — перед отправкой в удалённый репозиторий  

#### **Серверные (на GitHub/GitLab/etc)**  
Срабатывают на сервере:  
- `pre-receive` — перед принятием пуша  
- `update` — аналогично, но для каждой ветки отдельно  
- `post-receive` — после принятия пуша *(например, для деплоя)*  

---  

### **3. Практика: Создаём pre-commit хук**  

**Задача:** Проверить, нет ли в коде `console.log` перед коммитом.  

1. Переходим в папку `.git/hooks` вашего проекта:  
   ```bash
   cd .git/hooks
   ```

2. Создаём файл `pre-commit` (без расширения!):  
   ```bash
   touch pre-commit && chmod +x pre-commit
   ```

3. Пишем скрипт (пример на Bash):  
   ```bash
   #!/bin/sh

   if git diff --cached --name-only | xargs grep -n 'console.log'; then
     echo "ERROR: Найдены console.log в staged-файлах!"
     exit 1  # Отменяем коммит
   fi
   ```

**Как работает:**  
- Если находит `console.log` — коммит не пройдёт.  
- Чтобы обойти проверку (если очень нужно):  
  ```bash
  git commit --no-verify
  ```

---  

### **4. Серверный хук: post-receive**  

**Задача:** Автоматически деплоить код после пуша в `main`.  

1. На сервере в `bare`-репозитории:  
   ```bash
   cd /path/to/repo.git/hooks
   touch post-receive && chmod +x post-receive
   ```

2. Содержимое скрипта:  
   ```bash
   #!/bin/sh

   while read oldrev newrev refname; do
     if [ "$refname" = "refs/heads/main" ]; then
       echo "Деплой на production..."
       git --work-tree=/var/www/html --git-dir=/path/to/repo.git checkout -f
     fi
   done
   ```

**Важно:**  
- Хук работает на сервере (например, GitHub Actions, GitLab CI).  
- В реальных проектах лучше использовать CI/CD (GitHub Actions, Jenkins).  

---  

### **5. Conventional Commits + Автоматизация**  

#### **Что такое Conventional Commits?**  
Стандарт структуры сообщений коммитов:  
```
тип(опциональная область): описание

тело (необязательно)

нижний колонтитул (необязательно)
```  

**Пример:**  
```
feat(auth): add Google OAuth login

Closes #123
```  

#### **Как автоматизировать?**  
1. **commitizen** — интерактивный ввод коммитов:  
   ```bash
   npm install -g commitizen
   cz-conventional-changelog
   ```  
   Теперь вместо `git commit` используете `git cz`.  

2. **husky + commitlint** — проверка формата:  
   ```bash
   npx husky-init && npm install
   echo "module.exports = { extends: ['@commitlint/config-conventional'] }" > commitlint.config.js
   ```  
   Добавляем хук:  
   ```bash
   npx husky add .husky/commit-msg 'npx commitlint --edit "$1"'
   ```  

**Итог:** Теперь коммиты без правильного формата не пройдут!  

---  

### **Чего стоит избегать?**  
- **Слишком строгих хуков** (если они мешают работе).  
- **Хуков, которые долго выполняются** (например, запуск всех тестов на `pre-commit`).  
- **Серверных хуков без fallback-механизмов** (если хук упадёт — push может не пройти).  

---  

## **Итог**  
1. **Хуки** — это "стражи" вашего репозитория.  
2. **pre-commit** — для проверки кода перед коммитом.  
3. **post-receive** — для автоматического деплоя.  
4. **Conventional Commits + husky** — для единого стиля коммитов.  

