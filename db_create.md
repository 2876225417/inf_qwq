
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


CREATE TABLE rtsp_sources (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    ip VARCHAR(50) NOT NULL,
    port INTEGER NOT NULL,
    channel VARCHAR(50) NOT NULL,
    subtype VARCHAR(50),
    rtsp_url VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


CREATE INDEX idx_rtsp_sources_username ON rtsp_sources(username);
CREATE INDEX idx_rtsp_sources_ip ON rtsp_sources(ip);
CREATE INDEX idx_rtsp_sources_rtsp_url ON rtsp_sources(rtsp_url);
```