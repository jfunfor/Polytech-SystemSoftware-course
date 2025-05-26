# Тема Инструменты мониторинга. Задание 2. Перегрев в дата-центре Гелиос.

В кибер-инфраструктуре _Helios Data-Cube_ происходит перегрев: кластеры мониторинга получают метрики от 10 000 узлов, и система почти упала. Ваша задача — стабилизировать мониторинг и разгрузить Prometheus.

## Условие задания

Каталог `/helios_stack/` содержит:
- Примеры метрик.
- Конфиги Prometheus и Alertmanager.
- JSON-дэшборды Grafana.
- Алерты.

**Ваша задача:**
1. Создать файл `metrics_types_report.txt`, где перечислить все типы метрик, найденные в каталоге `metrics_samples/`, и для каждой привести один пример строки.
2. В файле `prometheus.yml`:
	- Найти `scrape_interval` менее 5s.
	- Изменить их на 15s и сохранить файл.
3. В дэшбордах из `grafana_dashboards/` найти панели, где более 3 target'ов (много метрик за раз). Их `id` записать в `heavy_panels.txt`.
4. В каждом файле из `alerts/` убедиться, что у алертов есть блоки `expr`, `for`, `labels`, `annotations`.
5. В `prometheus.yml` добавить комментарий `# Federation enabled` над любым `job_name`, как сигнал для архитектурной оптимизации.

## Советы

Используется следующая структура исходных файлов:
```
helios_stack/
├── prometheus.yml            # требует правки
├── alerts/
│   ├── cpu_alerts.yml        # изучить блоки
│   └── memory_alerts.yml     # изучить блоки
├── grafana_dashboards/
│   ├── dashboard_1.json      # найти тяжелые панели
│   └── dashboard_2.json      # найти тяжелые панели
├── metrics_samples/
│   ├── node1.prom            # найти все типы метрик
│   └── node2.prom            # найти все типы метрик
```

Примеры исходного содержимого:

`metrics_samples/node1.prom`:
```
# HELP cpu_usage_total Total CPU time
# TYPE cpu_usage_total counter
cpu_usage_total 1223.4
```

`prometheus.yml`:
```
global:
  scrape_interval: 15s

scrape_configs:
  - job_name: "node_fast"
    scrape_interval: 2s
    static_configs:
      - targets: ["localhost:9100"]

  - job_name: "node_slow"
    scrape_interval: 30s
    static_configs:
      - targets: ["localhost:9200"]
```

`grafana_dashboards/dashboard_1.json`:
```
{
  "panels": [
    { "id": 1, "targets": [ {"expr": "up"}, {"expr": "cpu"}, {"expr": "mem"}, {"expr": "disk"} ] },
    { "id": 2, "targets": [ {"expr": "uptime"} ] }
  ]
}
```

`alerts/cpu_alerts.yml`:
```
groups:
- name: cpu_alerts
  rules:
  - alert: HighCPU
    expr: avg(rate(cpu_usage_total[1m])) > 0.9
    for: 2m
    labels:
      severity: warning
    annotations:
      summary: "High CPU usage"
```

## Цели по теме Инструменты мониторинга

| Тема                                 | Применение                                                       |
| ------------------------------------ | ---------------------------------------------------------------- |
| Prometheus, основные экспортеры      | Настройка конфигов Prometheus                                    |
| Graphana, использование с Prometheus | Изучение примера использования Graphana в связке с Prometheus    |
| Типы метрик в Prometheus             | Изучение примеров метрик, нахождение всех типов                  |
| Перегрузка систем мониторинга        | Изменение `scrape_interval`, нахождение тяжелых панелей Graphana |

## Автоматическая проверка 

Обратите внимание на то, что будет проверяться автоматически:

1. Проверка файла с перечисленными метриками `metrics_types_report.txt`.
2. Проверка изменения `scrape_interval`.
3. Корректность расположения и наличие Federation комментария.
4. Проверка файла с id тяжелых панелей `heavy_panels.txt`.
5. Проверка полноты алертов.