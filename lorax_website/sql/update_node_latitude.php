<?php
    // Prepare SQL statement
    $sql = "UPDATE Node SET last_transmission = :last_transmission, gps_latitude = :latitude WHERE node_ID = :node_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':last_transmission', $Node->last_transmission);
    $statement->bindvalue(':latitude', $latitude_new);
    $statement->bindvalue(':node_ID', $node_ID);

    // Execute prepared SQL statement
    $statement->execute();
?>