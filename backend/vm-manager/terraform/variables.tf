variable "yc_token" {
  description = "IAM-токен для доступа к Yandex Cloud"
  type        = string
}

variable "yc_cloud_id" {
  description = "ID облака в Yandex Cloud"
  type        = string
}

variable "yc_folder_id" {
  description = "ID каталога (folder) в облаке"
  type        = string
}

variable "yc_zone" {
  description = "Зона доступности Yandex Cloud"
  type        = string
  default     = "ru-central1-a"  # можно установить значение по умолчанию
}

variable "yc_subnet_id" {
  description = "ID подсети для подключения ВМ"
  type        = string
}

variable "vm_name" {
  description = "Имя виртуальной машины"
  type        = string
}

variable "cpu_cores" {
  description = "Количество ядер CPU"
  type        = number
}

variable "memory_gb" {
  description = "Количество памяти в Гб"
  type        = number
}

variable "disk_size_gb" {
  description = "Размер диска в Гб"
  type        = number
}
