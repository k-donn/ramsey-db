from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
import sqlite3
import os

app = FastAPI()
DB_PATH = "ramsey_results.db"

def get_db_connection():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

@app.get("/db/{tbl}/{id}")
def read_row(tbl: str, id: int):
    conn = get_db_connection()
    cur = conn.cursor()
    query = f"SELECT * FROM {tbl} WHERE id = ?"
    cur.execute(query, (id,))
    row = cur.fetchone()
    conn.close()
    if row is None:
        raise HTTPException(status_code=404, detail="Row not found")
    return dict(row)


@app.get("/index.html")
def serve_index():
    file_path = os.path.join(os.path.dirname(__file__), "index.html")
    if not os.path.exists(file_path):
        raise HTTPException(status_code=404, detail="index.html not found")
    return FileResponse(file_path, media_type="text/html")