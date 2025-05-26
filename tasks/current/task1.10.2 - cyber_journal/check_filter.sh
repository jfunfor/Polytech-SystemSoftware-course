#!/bin/bash

# Цвета для вывода
GREEN="\e[32m"
RED="\e[31m"
YELLOW="\e[33m"
RESET="\e[0m"

# Основная функция автотеста
run_test() {
    input_file="$1"       # Файл с журналом
    expected_file="$2"    # Файл с ожидаемым результатом
    test_name="$3"        # Название теста (для отчёта)

    if [[ ! -f "$input_file" ]]; then
        echo -e "${YELLOW} Пропуск: файл $input_file не найден.${RESET}"
        return 1
    fi

    if [[ ! -f "$expected_file" ]]; then
        echo -e "${YELLOW} Пропуск: файл $expected_file не найден.${RESET}"
        return 1
    fi

    if [[ ! -x ./cyber_filter.sh ]]; then
        echo -e "${RED} Ошибка: скрипт ./cyber_filter.sh не исполняемый или не найден.${RESET}"
        return 1
    fi

    # Запуск скрипта и отбор нужных строк
    ./cyber_filter.sh "$input_file" > output.txt
    grep -E "AGENT-[0-9]{4}" output.txt > matches.txt

    # Сравнение вывода
    if diff -u matches.txt "$expected_file" > diff.log; then
        echo -e "${GREEN} PASSED${RESET}: $test_name"
    else
        echo -e "${RED} FAILED${RESET}: $test_name"
        echo -e "${YELLOW} Различия:${RESET}"
        cat diff.log
    fi

    # Очистка (по желанию)
    rm -f output.txt matches.txt diff.log
}