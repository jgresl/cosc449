CREATE DATABASE lorax;

GRANT ALL PRIVILEGES ON lorax.* TO 'web_user'@'%' IDENTIFIED BY 'web_pass';

CREATE TABLE Node (
    node_ID INT AUTO_INCREMENT,
    serial_number VARCHAR(130),
    last_transmission TIMESTAMP,
    gps_latitude DECIMAL(9,6) DEFAULT 49.859148,
    gps_longitude DECIMAL(9,6) DEFAULT -119.605174,
    PRIMARY KEY (node_id)
);

CREATE TABLE Sample (
    sample_ID INT AUTO_INCREMENT,
    node_ID INT,
    sensor_type CHAR(1),
    date_time TIMESTAMP,
    value INT,
    sample INT,
    PRIMARY KEY (sample_id),
    FOREIGN KEY (node_ID) REFERENCES Node(node_ID) ON DELETE NO ACTION ON UPDATE CASCADE
);