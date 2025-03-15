
```sql
CREATE TABLE warning_records (
    id SERIAL PRIMARY KEY,
    cam_id INTEGER NOT NULL,
    cam_name VARCHAR(100),
    inf_res TEXT NOT NULL,
    status BOOLEAN NOT NULL,
    keywords TEXT,
    rtsp_name VARCHAR(100),
    rtsp_url TEXT NOT NULL,
    record_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    push_status BOOLEAN DEFAULT FALSE,
    push_message TEXT
);
```