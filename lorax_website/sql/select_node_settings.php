<?php
    // Prepare SQL statement
    $sql = "SELECT last_transmission, sample_frequency AS sample_old, transmission_frequency AS transmit_old, gps_latitude AS latitude_old, gps_longitude AS longitude_old
            FROM Node
            WHERE node_ID = :node_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':node_ID', $node_ID);

    // Execute prepared SQL statement
    $statement->execute();
?>