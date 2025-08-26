import redis.asyncio as redis
import redis as redis_sync
from dotenv import load_dotenv
import asyncio
import os
from pathlib import Path
from ..db import get_db
from ..models import VM
from ..schemas import (
    VMStatus,
)

load_dotenv(Path(__file__).parent.parent / '.env')

# Асинхронный клиент для воркера
async_redis_client = redis.Redis(
    host=os.getenv("REDIS_HOST", "localhost"),
    port=int(os.getenv("REDIS_PORT", "6379")),
    db=0,
    decode_responses=True,
)

# Синхронный клиент для BackgroundTasks
sync_redis_client = redis_sync.Redis(
    host=os.getenv("REDIS_HOST", "localhost"),
    port=int(os.getenv("REDIS_PORT", "6379")),
    db=0,
    decode_responses=True,
)

VM_TTL_SECONDS = int(os.getenv("VM_TTL_SECONDS", 3600))


async def vm_cleanup_worker(tf_manager):
    """
    Воркер, который слушает события истечения ключей от Redis.
    """
    print("Воркер очистки VM (pub/sub) запущен", flush=True)
    
    pubsub = async_redis_client.pubsub()
    await pubsub.psubscribe('__keyevent@0__:expired')
    
    try:
        async for message in pubsub.listen():
            if message['type'] == 'pmessage':
                # message['data'] содержит ключ, который истек
                expired_key = message['data']
                print(f"Ключ истек через pub/sub: {expired_key}", flush=True)
                
                await handle_expired_vm(expired_key, tf_manager)
                
    except Exception as e:
        print(f"Ошибка в pub/sub воркере: {e}", flush=True)
    finally:
        await pubsub.close()

async def handle_expired_vm(vm_id: str, tf_manager):
    """Обработка истекшей VM"""
    try:
        print(f"Обрабатываем истекшую VM: {vm_id}", flush=True)
        
        tf_manager.delete_vm(vm_id)
        print(f"УДАЛЕНИЕ ВМ {vm_id}", flush=True)

        db = next(get_db())
        try:
            vm = db.get(VM, vm_id)
            if vm:
                vm.status = VMStatus.DELETED_BY_TIMER
                db.commit()
                print(f"Статус VM {vm_id} обновлен", flush=True)
            else:
                print(f"VM {vm_id} не найдена в БД", flush=True)
        finally:
            db.close()
            
    except Exception as e:
        print(f"Ошибка при обработке VM {vm_id}: {e}", flush=True)


def set_redis_ttl_for_vm(vm_id: str):
    """
    Установка ключа в Redis с TTL для управления временем жизни ВМ.
    Синхронная функция для использования в BackgroundTasks.
    """
    try:
        result = sync_redis_client.set(vm_id, "active", ex=VM_TTL_SECONDS)
        print(f"Установка ключа в Redis для VM {vm_id}: {result}", flush=True)
    except Exception as e:
        print(f"Ошибка при установке TTL для VM {vm_id}: {e}", flush=True)