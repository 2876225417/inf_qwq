```sql
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



CREATE TABLE IF NOT EXISTS inf_qwq_result (
        id SERIAL PRIMARY KEY, 
        cam_id INTEGER NOT NULL, 
        keywords TEXT, 
        inf_result TEXT, 
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)
```