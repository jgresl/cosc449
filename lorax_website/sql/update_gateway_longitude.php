<?php
    // Prepare SQL statement
    $sql = "UPDATE Gateway SET gps_longitude = :gps_longitude WHERE gateway_ID = :gateway_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':gps_longitude', $longitude_new);
    $statement->bindvalue(':gateway_ID', $gateway_ID);
    
    // Execute prepared SQL statement
    $statement->execute();
?>