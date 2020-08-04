<?php
    $node_ID = $_GET['node'];
    $sensor_type = $_GET['sensor'];

    // Prepare SQL statement
    $sql = "SELECT 
                sample_ID, 
                node_ID, 
                CASE 
                    WHEN sensor_type = 'H' THEN 'Humidity'
                    WHEN sensor_type = 'T' THEN 'Temperature'
                    WHEN sensor_type = 'B' THEN 'Battery'
                    WHEN sensor_type = 'P' THEN 'Air Pressure'
                    ELSE sensor_type
                END AS sensor_type,
                date_time,
                value, 
                sample
            FROM Sample
            WHERE node_ID = $node_ID AND sensor_type = '$sensor_type'
            ORDER BY sample_ID DESC";
            
    $statement = $pdo->prepare($sql);

    // Execute prepared SQL statement
    $statement->execute();
?>