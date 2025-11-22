CREATE TABLE IF NOT EXISTS DocumentChunks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    source_table TEXT NOT NULL,   
    source_id INTEGER NOT NULL,  
    chunk_index INTEGER NOT NULL, 
    text TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS VectorMapping (
    index_id INTEGER PRIMARY KEY, 
    chunk_id INTEGER NOT NULL,    
    metadata TEXT                
);

CREATE TABLE IF NOT EXISTS VectorMetadata (
    index_id INTEGER PRIMARY KEY,  
    source_table TEXT,           
    source_id INTEGER,             
    text TEXT                     
);