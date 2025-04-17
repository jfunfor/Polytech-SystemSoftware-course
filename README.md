# 1. Основные текстовые редакторы

## 1.1. nano

nano — это простой текстовый редактор, который работает в терминале. Он интуитивно понятен и подходит для быстрого редактирования файлов.

Команды:

• Открыть файл: nano имя_файла

• Сохранить изменения: Ctrl + O

• Выйти: Ctrl + X

## 1.2. vi/vim

vi и его улучшенная версия vim — это мощные текстовые редакторы, которые имеют более сложный интерфейс, но и гораздо больше возможностей.

Основные команды:

• Открыть файл: vi имя_файла

• Перейти в режим редактирования: нажмите i

• Сохранить изменения и выйти: :wq

• Выйти без сохранения: :q!

## 1.3. emacs

emacs — это еще один мощный текстовый редактор, который предлагает множество функций, включая поддержку плагинов и расширений.

Основные команды:

• Открыть файл: emacs имя_файла

• Сохранить файл: Ctrl + X, затем Ctrl + S

• Выйти: Ctrl + X, затем Ctrl + C

# 2. Команды для работы с текстом

## 2.1. cat

Команда cat используется для отображения содержимого текстового файла.

Пример использования:
cat имя_файла


## 2.2. grep

grep — это мощный инструмент для поиска текста в файлах. Он позволяет находить строки, соответствующие определенному шаблону.

Пример использования:
grep 'шаблон' имя_файла


## 2.3. sed

sed — это потоковый редактор, который позволяет выполнять преобразования текста на лету.

Пример замены текста:
sed 's/старый_текст/новый_текст/g' имя_файла


## 2.4. awk

awk — это язык программирования для обработки текстовых файлов и генерации отчетов.

Пример использования:
awk '{print $1}' имя_файла

Этот пример выведет первый столбец из файла.

# 3. Работа с файлами

## 3.1. Создание и удаление файлов

• Создать файл: 
touch имя_файла


• Удалить файл:
rm имя_файла


## 3.2. Копирование и перемещение файлов

• Копировать файл:
cp имя_файла имя_копии


• Переместить файл:
mv имя_файла новое_место


# 4. Форматирование текста

## 4.1. fmt

Команда fmt используется для форматирования текста в файлах.

Пример использования:
fmt имя_файла


## 4.2. fold

fold позволяет разбивать строки длиннее определенной длины.

Пример использования:
fold -w 80 имя_файла

Это разобьет строки, превышающие 80 символов.

# 5. Поиск и замена текста

## 5.1. find

Команда find позволяет искать файлы и директории по заданным критериям.

Пример использования:
find /путь/к/директории -name '*.txt'


## 5.2. xargs

xargs позволяет передавать аргументы из стандартного ввода в другую команду.

Пример использования:
find . -name '*.txt' | xargs grep 'шаблон'
