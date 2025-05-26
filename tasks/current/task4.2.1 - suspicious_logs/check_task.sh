#!/bin/bash

echo "Проверка задания..."

# 1. Проверка файла журнала
if [ ! -f /var/log/cyberwatch_incidents.log ]; then
  echo "Файл с инцидентами не найден"
  exit 1
fi

# 2. Проверка содержимого файла
if ! grep -q "cyberwatch" /var/log/cyberwatch_incidents.log; then
  echo "В файле нет записей о cyberwatch"
  exit 1
fi

# 3. Проверка конфигурации logrotate
if ! grep -q "/var/log/cyberwatch_incidents.log" /etc/logrotate.d/*; then
  echo "logrotate не настроен для cyberwatch_incidents.log"
  exit 1
fi

# 4. Проверка наличия сжатых логов
if ! ls /var/log/cyberwatch_incidents.log.*.gz >/dev/null 2>&1; then
  echo "Логи не ротируются или не сжимаются"
  exit 1
fi

# 5. Проверка настройки rsyslog
if ! grep -q "@10.0.0.2" /etc/rsyslog.conf && ! grep -q "@10.0.0.2" /etc/rsyslog.d/*; then
  echo "Rsyslog не настроен на отправку логов"
  exit 1
fi

echo "Все проверки пройдены. Задание выполнено!"