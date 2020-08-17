<?php
    // Prepare SQL statement
    $sql = "UPDATE Gateway SET gps_latitude = :gps_latitude WHERE gateway_ID = :gateway_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':gps_latitude', $latitude_new);
    $statement->bindvalue(':gateway_ID', $gateway_ID);
    
    // Execute prepared SQL statement
    $statement->execute();
?>