import sys
import json
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, BASE_DIR)

from rag_module import answer_question

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({"error": "Нет вопроса"}, ensure_ascii=False))
        sys.exit(1)

    question = sys.argv[1]

    try:
        answer = answer_question(question)
        print(json.dumps({"answer": answer}, ensure_ascii=False))
    except Exception as e:
        print(json.dumps({"error": str(e)}, ensure_ascii=False))
        sys.exit(1)
