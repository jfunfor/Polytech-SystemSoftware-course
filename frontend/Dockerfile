FROM frappe/bench:latest
RUN bench init --skip-redis-config-generation frappe-bench
WORKDIR frappe-bench
RUN <<EOF 
bench set-redis-cache-host redis://192.168.1.135:6379
bench set-redis-queue-host redis://192.168.1.135:6379
bench set-redis-socketio-host redis://192.168.1.135:6379
sed -i '/redis/d' ./Procfile && sed -i '/watch/d' ./Procfile
bench get-app lms https://github.com/jfunfor/frappe-lms.git
EOF

RUN <<EOF
bench new-site lms.localhost --force --mariadb-root-password root --admin-password admin --db-host=192.168.1.135
EOF

RUN <<EOF
bench --site lms.localhost install-app lms
bench --site lms.localhost set-config developer_mode 1
bench --site lms.localhost clear-cache
bench use lms.localhost
EOF

CMD sleep infinity
