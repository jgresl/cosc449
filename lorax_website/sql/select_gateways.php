<?php
    // Prepare SQL statement
    $sql = "SELECT gateway_ID, gateway_description, gps_latitude, gps_longitude
            FROM Gateway
            ORDER BY gateway_ID ASC";
            
    $statement = $pdo->prepare($sql);

    // Execute prepared SQL statement
    $statement->execute();
?>