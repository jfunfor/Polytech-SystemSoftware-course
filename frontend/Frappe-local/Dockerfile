FROM frappe/bench:latest

WORKDIR /home/frappe/frappe-bench
COPY . .

RUN pip install -r requirements.txt

RUN bench build --production

RUN bench new-site learninglinux.test --mariadb-root-password 123 --admin-password admin --install-app lms

CMD ["bench", "start"]
