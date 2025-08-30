variable "proxmox_api_url" {
  description = "IAM-токен для доступа к Yandex Cloud"
  type        = string
}

variable "proxmox_api_token_id" {
  description = "ID облака в Yandex Cloud"
  type        = string
}

variable "proxmox_api_token_secret" {
  description = "ID каталога (folder) в облаке"
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

variable "vm_user"{
  description = "Пользователь ssh"
  type        = string
}

variable "vm_password"{
  description = "Пароль ssh"
  type        = string
  sensitive   = true

}
