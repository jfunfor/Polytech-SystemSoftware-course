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

if os.getenv("DOCKER_ENV") != "true":
    load_dotenv(Path(__file__).parent.parent / '.env')

# Асинхронный клиент для воркера
async_redis_client = redis.Redis(
    host=os.getenv("REDIS_HOST", "localhost"),
    port=int(os.getenv("REDIS_PORT", "6379")),
    db=1,
    password=os.getenv("REDIS_PASS"),
    decode_responses=True,
)

# Синхронный клиент для BackgroundTasks
sync_redis_client = redis_sync.Redis(
    host=os.getenv("REDIS_HOST", "localhost"),
    port=int(os.getenv("REDIS_PORT", "6379")),
    db=1,
    password=os.getenv("REDIS_PASS", "localhost"),
    decode_responses=True,
)

VM_TTL_SECONDS = int(os.getenv("VM_TTL_SECONDS", 3600))


async def vm_cleanup_worker(tf_manager):
    """
    Воркер, который слушает события истечения ключей от Redis.
    """
    print("Воркер очистки VM (pub/sub) запущен", flush=True)
    
    pubsub = async_redis_client.pubsub()
    await pubsub.psubscribe('__keyevent@1__:expired')
    
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

async def delete_redis_key(vm_id: str):
    """
    Удаление ключа VM из Redis при ручном удалении
    """
    try:
        # Удаляем ключ из Redis
        result = await async_redis_client.delete(vm_id)
        print(f"Ключ VM {vm_id} удален из Redis: {result}", flush=True)
        return result
    except Exception as e:
        print(f"Ошибка при удалении ключа {vm_id} из Redis: {e}", flush=True)
        raise

def delete_redis_key_sync(vm_id: str):
    """
    Синхронная версия для использования в фоновых задачах
    """
    try:
        result = sync_redis_client.delete(vm_id)
        print(f"Ключ VM {vm_id} удален из Redis (sync): {result}", flush=True)
        return result
    except Exception as e:
        print(f"Ошибка при удалении ключа {vm_id} из Redis (sync): {e}", flush=True)
        raise

async def extend_redis_ttl(vm_id: str):
    """
    Продление TTL ключа VM в Redis при активности пользователя
    Если ключа нет - ничего не делаем
    """
    try:
        # Проверяем существует ли ключ
        exists = await async_redis_client.exists(vm_id)
        if exists:
            # Продлеваем TTL на стандартное время
            result = await async_redis_client.expire(vm_id, VM_TTL_SECONDS)
            print(f"TTL для VM {vm_id} продлен: {result}", flush=True)
            return result
        else:
            # Если ключа нет, просто выводим сообщение и ничего не делаем
            print(f"Ключ для VM {vm_id} не найден в Redis, продление не требуется", flush=True)
            return False
    except Exception as e:
        print(f"Ошибка при продлении TTL для VM {vm_id}: {e}", flush=True)
        raise

def extend_redis_ttl_sync(vm_id: str):
    """
    Синхронная версия для продления TTL
    Если ключа нет - ничего не делаем
    """
    try:
        exists = sync_redis_client.exists(vm_id)
        if exists:
            result = sync_redis_client.expire(vm_id, VM_TTL_SECONDS)
            print(f"TTL для VM {vm_id} продлен (sync): {result}", flush=True)
            return result
        else:
            print(f"Ключ для VM {vm_id} не найден в Redis (sync), продление не требуется", flush=True)
            return False
    except Exception as e:
        print(f"Ошибка при продлении TTL для VM {vm_id} (sync): {e}", flush=True)
        raise