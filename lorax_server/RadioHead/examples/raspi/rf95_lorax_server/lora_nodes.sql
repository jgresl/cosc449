CREATE DATABASE lorax;

GRANT ALL PRIVILEGES ON lorax.* TO 'web_user'@'%' IDENTIFIED BY 'web_pass';

CREATE TABLE Node (
    node_ID INT AUTO_INCREMENT,
    serial_number VARCHAR(150),
    sync_status INT DEFAULT 1,
    last_sync TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_transmission TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    sample_frequency INT DEFAULT 15,
    transmission_frequency INT DEFAULT 60,
    gps_latitude DECIMAL(9,6),
    gps_longitude DECIMAL(9,6),
    PRIMARY KEY (node_id)
);

CREATE TABLE Sample (
    sample_ID INT AUTO_INCREMENT,
    node_ID INT,
    sensor_type CHAR(1),
    date_time TIMESTAMP,
    value DECIMAL(8,2),
    sample INT,
    PRIMARY KEY (sample_id),
    FOREIGN KEY (node_ID) REFERENCES Node(node_ID) ON DELETE NO ACTION ON UPDATE CASCADE
);