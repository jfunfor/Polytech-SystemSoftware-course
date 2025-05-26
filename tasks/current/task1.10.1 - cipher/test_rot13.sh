#!/bin/bash

# Цвета для вывода
GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

# Проверяем, что скрипт существует
SCRIPT="./rot13.sh"
if [[ ! -x "$SCRIPT" ]]; then
    echo -e "${RED}Ошибка: не найден или неисполняемый файл $SCRIPT${RESET}"
    exit 1
fi

# Тестовые пары: "ввод:ожидаемый результат"
tests=(
  "Cyberpunk:Plorelchak"
  "NightCity:AvtugPvgl"
  "HackTheWorld:UnpxGurJbeyq"
  "rot13:ebg13"
  "SecretMessage:FrpergZrffntr"
)

# Счётчики
passed=0
total=${#tests[@]}

echo "Запуск автотестов для ROT13..."

for test in "${tests[@]}"; do
    input="${test%%:*}"
    expected="${test##*:}"

    # Получаем фактический вывод скрипта (ищем строку с зашифрованным сообщением)
    output=$(echo "$input" | "$SCRIPT")
    actual=$(echo "$output" | grep -i "зашифрован" | sed -E 's/.*зашифрованное сообщение:? *//I')

    if [[ "$actual" == "$expected" ]]; then
        echo -e "${GREEN}ПАСSED${RESET}: '$input' → '$actual'"
        ((passed++))
    else
        echo -e "${RED}FAILED${RESET}: '$input'\n    → Ожидалось: '$expected', Получено: '$actual'"
    fi
done

echo
echo "Результат: $passed из $total тестов пройдено."

# Возврат кода ошибки, если есть провалы
if (( passed < total )); then
    exit 1
else
    exit 0
fi
