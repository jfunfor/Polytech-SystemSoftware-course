# Руководство по Bash-скриптам

Bash — это командный интерпретатор, широко используемый в Unix-подобных системах для автоматизации задач. В этом руководстве рассмотрим основные элементы Bash-скриптов: циклы, условные выражения, чтение с клавиатуры, аргументы и массивы.

---

## Циклы (Loops)

Циклы позволяют выполнять команды многократно.

**Цикл for:**
```bash
#!/bin/bash
for i in {1..5}
do
  echo "Число: $i"
done
```

**Цикл while:**
```bash
#!/bin/bash
count=1
while [ $count -le 5 ]
do
  echo "Счетчик: $count"
  ((count++))
done
```

---

## Условные выражения (Conditional Statements)

Условные выражения позволяют выполнять команды по определённым условиям.

**Пример if-else:**
```bash
#!/bin/bash
echo "Введите число:"
read num

if [ $num -gt 10 ]; then
  echo "Число больше 10"
elif [ $num -eq 10 ]; then
  echo "Число равно 10"
else
  echo "Число меньше 10"
fi
```

---

## Чтение с клавиатуры (Input from Keyboard)

Для получения данных от пользователя используется команда `read`.

**Пример:**
```bash
#!/bin/bash
echo "Введите ваше имя:"
read name
echo "Привет, $name!"
```

---

## Аргументы (Arguments)

Аргументы позволяют передавать данные скрипту при его запуске.

**Пример использования аргументов:**
```bash
#!/bin/bash
echo "Первый аргумент: $1"
echo "Второй аргумент: $2"
echo "Всего аргументов: $#"
```

Запуск скрипта с аргументами:
```bash
./script.sh аргумент1 аргумент2
```

---

## Массивы (Arrays)

Массивы позволяют хранить несколько значений в одной переменной.

**Создание и работа с массивом:**
```bash
#!/bin/bash
fruits=("яблоко" "банан" "вишня")

# Вывести все элементы массива
echo "Все фрукты: ${fruits[@]}"

# Вывести первый элемент массива
echo "Первый фрукт: ${fruits[0]}"

# Перебрать массив циклом
for fruit in "${fruits[@]}"
do
  echo "Фрукт: $fruit"
done
```

---

Теперь вы знаете основы написания Bash-скриптов и можете использовать эти знания для автоматизации задач в Linux.
