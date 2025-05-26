#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Подготовка командной строки и имитация ввода пользователя
    const char *script = "./neon_market.sh 5000";

    // Здесь заранее подготовленные действия пользователя
    const char *input_sequence =
        "1\n"   // выбор товара: Нейроинтерфейс
        "1\n"   // количество: 1
        "y\n"   // продолжить
        "3\n"   // выбор: Усилитель слуха
        "1\n"   // количество: 1
        "n\n";  // завершить

    // Запуск скрипта через pipe
    FILE *fp = popen(script, "w+");
    if (!fp) {
        perror("Ошибка запуска скрипта");
        return 1;
    }

    // Передаём ввод
    fputs(input_sequence, fp);
    fflush(fp);

    // Читаем вывод
    char buffer[1024];
    int found_total = 0;
    int found_item1 = 0, found_item2 = 0;
    int found_balance = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer); // Для отладки

        if (strstr(buffer, "Нейроинтерфейс ×1")) found_item1 = 1;
        if (strstr(buffer, "Усилитель слуха ×1")) found_item2 = 1;
        if (strstr(buffer, "Оставшиеся кибер-кредиты: 1100")) found_balance = 1;
        if (strstr(buffer, "Итоговый заказ:")) found_total = 1;
    }

    pclose(fp);

    // Проверка
    if (found_total && found_item1 && found_item2 && found_balance) {
        printf("\nТест пройден успешно\n");
        return 0;
    } else {
        printf("\nТест провален\n");
        if (!found_total) puts(" - Не найден итоговый блок с заказом.");
        if (!found_item1) puts(" - Отсутствует Нейроинтерфейс ×1.");
        if (!found_item2) puts(" - Отсутствует Усилитель слуха ×1.");
        if (!found_balance) puts(" - Остаток кредитов некорректен.");
        return 1;
    }
}