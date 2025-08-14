output "vm_ip_address" {
  description = "IP-адрес созданной ВМ"
  value       = proxmox_vm_qemu.vm.default_ipv4_address
  depends_on  = [proxmox_vm_qemu.vm]
}