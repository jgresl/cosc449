<?php
    // Prepared SQL statement to return all samples with the newest on top
    $sql = "SELECT node_ID, sensor_type, date_time, value, sample
            FROM Sample
            ORDER BY date_time DESC";
            
    $statement = $pdo->prepare($sql);
?>