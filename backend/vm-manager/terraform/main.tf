resource "proxmox_vm_qemu" "vm" {
  name        = var.vm_name
  target_node   = "proxmox301"
  clone       = "vm-manager-template"  
  agent       = 1
  full_clone  = true
  scsihw      = "virtio-scsi-pci"

  cores   = var.cpu_cores
  memory  = var.memory_gb

  disks{
    ide {
            ide2 {
                cloudinit {
                    storage = "local-lvm"
                }
            }
        }
    scsi {
              scsi0 {
                  disk {
                      size            = "${var.disk_size_gb}G"
                      storage         = "local-lvm"
                      replicate       = true
                  }
              }
          }
  }

  network {
    model  = "virtio"
    bridge = "vmbr0"
  }

  os_type    = "cloud-init"
  ciuser     = var.vm_user
  cipassword = var.vm_password

  ipconfig0  = "ip=dhcp"

  boot = "order=scsi0"
}