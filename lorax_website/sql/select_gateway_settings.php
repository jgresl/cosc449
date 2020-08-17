<?php
    // Prepare SQL statement
    $sql = "SELECT gateway_description AS description_old, gps_latitude AS latitude_old, gps_longitude AS longitude_old
            FROM Gateway
            WHERE gateway_ID = :gateway_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':gateway_ID', $gateway_ID);

    // Execute prepared SQL statement
    $statement->execute();
?>