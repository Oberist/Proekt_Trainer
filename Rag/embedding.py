"""
Модуль для создания эмбеддингов из текстового контекста тренировок (ONNX версия)
"""
import os
import sqlite3
import pickle
import numpy as np
import onnxruntime as ort
from transformers import AutoTokenizer

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(BASE_DIR, "Database", "TrainingDiaryVector.db")
TABLE = "TrainingContext"

# Папка с ONNX моделью
MODEL_DIR = os.path.join(BASE_DIR, "onnx_model")

print(f"[INFO] DB_PATH в embedding.py: {DB_PATH}")
print(f"[INFO] MODEL_DIR: {MODEL_DIR}")

# Загружаем токенайзер
tokenizer = AutoTokenizer.from_pretrained(MODEL_DIR)

# Загружаем ONNX модель
session = ort.InferenceSession(
    os.path.join(MODEL_DIR, "model.onnx"),
    providers=["CPUExecutionProvider"]
)

print("[INFO] ONNX модель и токенайзер загружены.")


def encode_text(text: str) -> np.ndarray:
    """Создаёт эмбеддинг через ONNX Runtime"""
    inputs = tokenizer(
        text,
        return_tensors="np",
        padding=True,
        truncation=True,
        max_length=256
    )

    outputs = session.run(None, {
        "input_ids": inputs["input_ids"],
        "attention_mask": inputs["attention_mask"]
    })

    last_hidden = outputs[0]          # [batch, seq, hidden]
    embedding = last_hidden.mean(axis=1)[0]  # усреднение по токенам

    return embedding.astype(np.float32)


def create_embeddings():
    """Создаёт эмбеддинги для всех текстов без эмбеддингов"""
    if not os.path.exists(DB_PATH):
        raise FileNotFoundError(f"БД не найдена: {DB_PATH}")

    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()

    cur.execute(f"SELECT id, context_text FROM {TABLE} WHERE embedding IS NULL")
    rows = cur.fetchall()

    print(f"[INFO] Найдено {len(rows)} записей без эмбеддингов")

    for row_id, text in rows:
        emb = encode_text(text)
        emb_bytes = pickle.dumps(emb)

        cur.execute(
            f"UPDATE {TABLE} SET embedding = ? WHERE id = ?",
            (emb_bytes, row_id)
        )

    conn.commit()
    conn.close()
    print("[INFO] Эмбеддинги успешно созданы.")


def get_embeddings_for_search(query: str) -> np.ndarray:
    """Эмбеддинг для поискового запроса"""
    return encode_text(query)
