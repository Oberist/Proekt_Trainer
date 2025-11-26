import sys
import json
import os
from sentence_transformers import SentenceTransformer

MODEL_NAME = "sentence-transformers/all-MiniLM-L6-v2"

model = SentenceTransformer(MODEL_NAME)

def get_embedding(text: str):
    vec = model.encode(text, normalize_embeddings=False)
    return vec.tolist()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("[]")
        sys.exit(0)

    input_file = sys.argv[1].strip()
    if input_file.startswith('"') and input_file.endswith('"'):
        input_file = input_file[1:-1]

    if not os.path.exists(input_file):
        print("[]")
        sys.exit(0)

    try:
        with open(input_file, "r", encoding="utf-8") as f:
            text = f.read().strip()
    except:
        print("[]")
        sys.exit(0)

    if not text:
        print("[]")
        sys.exit(0)

    try:
        emb = get_embedding(text)
    except Exception:
        print("[]")
        sys.exit(0)

    print(json.dumps(emb), flush=True)