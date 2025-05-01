from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
import sqlite3
import os

app = FastAPI()
DB_PATH = "ramsey_results.db"
TABLE_NAME = "r34"

def get_db_connection():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

@app.get("/db/{id}")
def read_row(id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute(f"SELECT * FROM {TABLE_NAME} WHERE id = ?", (id,))
    row = cur.fetchone()
    conn.close()
    if row is None:
        raise HTTPException(status_code=404, detail="Row not found")
    return dict(row)

@app.get("/db/stats")
def db_stats():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute(f"""
        SELECT 
            AVG(k4_count) as avg_k4,
            AVG(k3_count) as avg_k3,
            COUNT(k4_count) as nonempty_k4,
            COUNT(k3_count) as nonempty_k3
        FROM {TABLE_NAME}
        WHERE k4_count IS NOT NULL OR k3_count IS NOT NULL
    """)
    stats = cur.fetchone()
    conn.close()
    return {
        "avg_k4": stats["avg_k4"],
        "avg_k3": stats["avg_k3"],
        "nonempty_k4": stats["nonempty_k4"],
        "nonempty_k3": stats["nonempty_k3"]
    }

@app.get("/index.html")
def serve_index():
    file_path = os.path.join(os.path.dirname(__file__), "index.html")
    if not os.path.exists(file_path):
        raise HTTPException(status_code=404, detail="index.html not found")
    return FileResponse(file_path, media_type="text/html")