<?php
    // Prepare SQL statement
    $sql = "SELECT node_ID, node_description, serial_number, CASE WHEN sync_status = 1 THEN 'Synced' ELSE 'Required' END AS sync_status, last_sync, last_transmission, sample_frequency, transmission_frequency, gps_latitude, gps_longitude,
                CASE 
                    WHEN TIMESTAMPDIFF(MINUTE, last_transmission, NOW()) <= transmission_frequency THEN 'ON'
                    WHEN TIMESTAMPDIFF(MINUTE, last_transmission, NOW()) > transmission_frequency THEN 'OFF'
                    ELSE '?'
                END AS status
            FROM Node
            ORDER BY node_ID ASC";
            
    $statement = $pdo->prepare($sql);

    // Execute prepared SQL statement
    $statement->execute();
?>