# Course dev



Linux – это многопользовательская операционная система, которая использует концепцию Unix о привилегиях для обеспечения безопасности на уровне файловой системы. 
Существуют следующие привилегии:
1.	Право на чтение файла. Позволяет просматривать содержимое файла.  
2.	Право на изменение файла. Даёт возможность изменять содержимое файла или удалить его.  
3.	Право на запуск. Позволяет запускать обычный файл, но для этого также нужны права на чтение данного файла.  
В Linux каждый файл принадлежит одному пользователю и одной группе и имеет индивидуальные настройки доступа.   

•	Пользователи: Каждый пользователь в Linux имеет уникальное имя и идентификатор (UID). Пользователи могут быть обычными или системными. Обычные пользователи выполняют интерактивные задачи, в то время как системные пользователи запускают фоновые процессы и сервисы.

•	Группы: Группы — это коллекции пользователей, которые имеют общие права доступа к файлам и каталогам. Каждая группа имеет уникальное имя и идентификатор (GID). Пользователи могут входить в несколько групп одновременно.

Пароли


Пароли используются для аутентификации пользователей в системе. В Linux пароли хранятся в зашифрованном виде в файле /etc/shadow. Только суперпользователь (root) может изменить пароли других пользователей без их текущего пароля.
sudo (Super User DO) — это команда, которая позволяет обычным пользователям выполнять команды с привилегиями суперпользователя (root). Это позволяет администраторам делегировать определенные административные задачи другим пользователям без предоставления им полного доступа к системе как root.

Настройка sudo

Чтобы настроить sudo, необходимо отредактировать файл /etc/sudoers. Это можно сделать с помощью команды visudo, которая запускает редактор с проверкой синтаксиса после сохранения изменений.
1.	Добавление пользователя в sudoers:
   
•	Откройте файл /etc/sudoers с помощью visudo.
•	Добавьте строку в формате:

username ALL=(ALL:ALL) ALL

Где username — имя пользователя, которому вы хотите предоставить права sudo.

2.	Ограничение привилегий:
   
•	Можно ограничить привилегии пользователя, указав конкретные команды, которые он может выполнять:

username ALL=(ALL:ALL) NOPASSWD:/path/to/command

•	Это позволяет пользователю запускать только указанную команду с привилегиями root.

3.	Группы sudo:
   
•	Если нужно предоставить права sudo группе пользователей, добавить группу в файл /etc/sudoers:

groupname ALL=(ALL:ALL) ALL

groupname — имя группы, которой вы хотите предоставить права sudo.\





Примеры Использования sudo

•	Выполнение команды с привилегиями root:


sudo apt update


•	Вход в систему как root:


sudo su


Cуперпользователь

Кроме основных двух типов пользователей (системных и обычных) в Linux существует также суперпользователь, или root. Этот пользователь может переопределять права других пользователей на файлы и ограничивать доступ. По сути, суперпользователь имеет права доступа ко всем файлам, существующим на сервере. Этот пользователь имеет право вносить общесистемные изменения, потому его безопасность очень важна для безопасности самого сервера.
Также можно настроить других пользователей для выполнения полномочий суперпользователя. Для этого нужно открыть обычному пользователю доступ к команде sudo.

Типы файлов
В Linux существует два основных типа файлов: обычные и специальные.
Обычные файлы определяются символом дефиса в поле типа. Обычные файлы – это простые файлы, в которых можно хранить данные.
Файл является специальным, если в поле типа содержится буква. Такие файлы обрабатываются операционной системой иначе, чем обычные файлы. Символ в поле типа файла указывает, к какому виду специальных файлов относится  определенный файл. Самым распространённым видом специальных файлов является каталог (или директория), он определяется символом d . 

Классы доступа

Каждый файл имеет настройки для трёх категорий (или классов) доступа:
1.	пользователя – владельца файла;
2.	группы – владельца файла (к этому классу относятся все члены группы, в которой состоит пользователь – владелец файла);
3.	и остальных пользователей (к этому классу относятся все пользователи, которые не принадлежат к предыдущим классам).
Такого порядка привилегий придерживаются все дистрибутивы Linux.


Виды доступа
Каждая категория имеет три вида доступа, каждый из которых обозначается отдельным символом:

•	r (read) – право на чтение файла,

•	w (write) – право на изменение (редактирование),

•	x (execute) – право на запуск (в некоторых особых случаях вместо х может использоваться другой символ).
Если вместо символа стоит дефис, значит, данный класс не имеет соответствующего права доступа. 

Например:

drwxrwx—
В таком случае пользователь и группа – владельцы файла имеют все права доступа, а для остальных пользователей доступ к файлу полностью закрыт.


Право на чтение файла

Право на чтение обычного файла – это возможность просматривать его содержимое.
Право на чтение каталога позволяет пользователю просматривать имена файлов, хранящихся в нём.

Право на изменение файла

В случае с обычным файлом это право даёт пользователю возможность изменять содержимое файла или удалить его.
Если у пользователя есть право на изменение каталога, то такой пользователь может удалить каталог, изменять его содержимое (то есть, создавать, переименовывать или удалять файлы в нём) и содержимое хранящихся в нём файлов.

Право на запуск

Имея такое право, пользователь может запускать обычный файл, но для этого ему также нужны права на чтение данного файла. Права на запуск нужно устанавливать для исполняемых программ и сценариев оболочки.
Если у пользователя есть право на запуск каталога, то он может читать такой каталог, а также получить доступ к его метаданным о хранящихся файлах.

Примеры привилегий

•	-rw——-: доступ к файлу имеет только пользователь-владелец файла.

•	-rwxr-xr-x: любой пользователь системы может запустить такой файл.

•	-rw-rw-rw-: любой пользователь системы имеет право на изменение такого файла.

•	drwxr-xr-x: любой пользователь системы может читать данный каталог.

•	drwxrwx—: содержимое каталога могут читать и изменять только пользователь и группа-владелец.

Пользователь-владелец файла, как правило, обладает всеми правами на файл (или имеет больше прав, чем остальные классы). Из соображений безопасности файлы должны быть доступны только для тех пользователей, которым необходим доступ к ним по определенной причине.

