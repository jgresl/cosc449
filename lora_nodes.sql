CREATE DATABASE lorax;

GRANT ALL PRIVILEGES ON lorax.* TO 'web_user'@'%' IDENTIFIED BY 'web_pass';

CREATE TABLE Gateway (
    gateway_ID INT AUTO_INCREMENT,
    gateway_description VARCHAR(20),
    gps_latitude DECIMAL (9,6),
    gps_longitude DECIMAL (9,6),
    PRIMARY KEY (gateway_ID)
);

CREATE TABLE Node (
    node_ID INT AUTO_INCREMENT,
    node_description VARCHAR(20),
    gps_latitude DECIMAL(9,6),
    gps_longitude DECIMAL(9,6),
    serial_number VARCHAR(150),
    sync_status INT DEFAULT 1,
    last_sync TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_transmission TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    sample_frequency INT DEFAULT 10,
    transmission_frequency INT DEFAULT 30,
    gateway_ID INT DEFAULT 1,
    PRIMARY KEY (node_id),
    FOREIGN KEY (gateway_ID) REFERENCES Gateway(gateway_ID) ON DELETE NO ACTION ON UPDATE CASCADE
);

CREATE TABLE Sample (
    sample_ID INT AUTO_INCREMENT,
    sensor_type CHAR(1),
    date_time TIMESTAMP,
    value DECIMAL(8,2),
    sample INT,
    node_ID INT,
    PRIMARY KEY (sample_id),
    FOREIGN KEY (node_ID) REFERENCES Node(node_ID) ON DELETE NO ACTION ON UPDATE CASCADE
);

INSERT INTO Gateway (gateway_description) VALUES ('Gateway 1');