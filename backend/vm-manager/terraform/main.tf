resource "yandex_compute_instance" "vm" {
  name = var.vm_name
  platform_id = "standard-v1"
  zone = var.yc_zone
  resources {
    cores  = var.cpu_cores
    memory = var.memory_gb
  }

  boot_disk {
    initialize_params {
      image_id = "fd800c7s2p483i648ifv"  # Ubuntu 20.04
      size = var.disk_size_gb
    }
  }

  network_interface {
    subnet_id = var.yc_subnet_id
    nat       = true
  }

  metadata = {
    user-data = file("${path.module}/cloud-init.yml")
  }
}