# Проверка зомби
zombies=$(ps -eo stat | grep -c Z)
if [ "$zombies" -gt 0 ]; then
  echo "Остались зомби: $zombies"
else
  echo "Зомби уничтожены"
fi

# Проверка процесса
pgrep -f rogue_ai.sh > /dev/null && echo "rogue_ai всё ещё жив" || echo "rogue_ai уничтожен"

# Проверка сигнала USR1
grep -q "USR1" rogue_ai_fixed.sh && echo "trap на USR1 найден" || echo "trap на USR1 не реализован"

# Проверка отчета
if grep -q "SIGCHLD" mission_report.md && grep -q "wait" mission_report.md; then
  echo "Отчёт содержит анализ"
else
  echo "Отчёт неполный"
fi