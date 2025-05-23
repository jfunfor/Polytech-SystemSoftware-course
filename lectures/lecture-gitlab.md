# Course dev


# 🚀 **GitLab: Простая установка и настройка на Ubuntu**

---

## **Что такое GitLab?**

**GitLab** - это удобная платформа для командной работы с кодом, управления проектами и автоматизации.  
Здесь можно:

- Хранить и управлять репозиториями
- Отслеживать задачи и баги
- Обсуждать изменения через Merge Requests
- Настраивать сборку и деплой
- Вести документацию

> **GitLab - это не просто хранилище кода, а целый центр управления проектами!**

---

## **Возможности GitLab**

- **Репозитории Git** - храните и отслеживайте изменения в коде
- **Веб-интерфейс и CLI** - работайте через браузер или терминал
- **Merge Requests** - обсуждайте и принимайте изменения
- **Контроль доступа** - настраивайте права участников
- **Трекер задач** - управляйте задачами и багами
- **Wiki** - пишите документацию прямо в GitLab

**Две версии:**
- Community Edition (CE) - бесплатная
- Enterprise Edition (EE) - платная, с расширениями

---

## **Требования к серверу**

- **CPU:** 2 ядра и выше
- **RAM:** от 8 ГБ
- **Диск:** SSD, минимум 50 ГБ
- **Интернет:** стабильное соединение

---

## **Установка GitLab на Ubuntu 20.04**

> Все команды выполняются с `sudo`.

### **1. Обновите пакеты**
обновление sudo apt



### **2. Установите зависимости**
sudo apt install -y curl openssh-server ca-сертификаты tzdata perl
sudo apt install -y postfix



### **3. Добавьте репозиторий GitLab**
curl https://packages.gitlab.com/install/repositories/gitlab/gitlab-ce/script.deb.sh | sudo bash



### **4. Установите GitLab**
sudo apt устанавливает gitlab-ce



---

## **Первичная настройка GitLab**

### **1. Придумайте пароль администратора**
При первом входе задайте пароль для пользователя `root`.

---

### **2. Настройте внешний адрес**

Откройте файл:
sudo nano /etc/gitlab/gitlab.rb


Найдите строку:
внешний_адрес_или_ip_адрес


Замените `domain_or_ip` на свой домен или IP.

Примените изменения:
sudo gitlab-перенастройка ctl



---

### **3. Настройте почту (SMTP)**

В файле `/etc/gitlab/gitlab.rb` найдите и измените параметры:

gitlab_rails['smtp_enable'] = true

gitlab_rails['smtp_address'] = "smtp.example.ru "

gitlab_rails['smtp_port'] = 587

gitlab_rails['smtp_user_name'] = "user@example.ru "

gitlab_rails['smtp_password'] = "your_smtp_password"

gitlab_rails['smtp_domain'] = "example.ru "

gitlab_rails['smtp_authentication'] = "вход"

gitlab_rails['smtp_enable_starttls_auto'] = true



Примените настройки:

sudo gitlab-перенастройка ctl



---

### **4. Откройте нужные порты в брандмауэре**

sudo ufw разрешить OpenSSH

sudo ufw разрешить http

sudo ufw разрешить https




---

## **Работа через веб-интерфейс GitLab**

### **1. Первый вход**

Откройте в браузере:  

`http://gitlab.example.ru` (замените на свой домен или IP)

Введите:

- **username:** `root`
  
- **password:** ваш новый пароль
  

Нажмите **Sign In**

---

### **2. Настройте профиль**

- Перейдите: User Icon (справа вверху) → Settings
- Измените Name и Email
- Нажмите **Update Profile settings**
- Подтвердите e-mail через письмо

---

### **3. Смените имя пользователя**

- В меню слева выберите **Account**
- Найдите **Change username**, введите новое имя, нажмите **Update username**

---

### **4. Добавьте SSH-ключ**

Проверьте, есть ли SSH-ключ:

cat ~/.ssh/id_rsa.pub


Если нет - создайте:

ssh-кейген


Скопируйте содержимое:

cat ~/.ssh/id_rsa.pub


В GitLab:
- **Settings → SSH Keys**
- Вставьте ключ, введите название (Title), нажмите **Add key**

---

## **Готово!**



Для подробностей -  [официальная документация GitLab](https://docs.gitlab.com/).

---
