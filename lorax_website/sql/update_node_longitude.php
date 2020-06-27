<?php
    // Prepare SQL statement
    $sql = "UPDATE Node SET last_transmission = :last_transmission, gps_longitude = :longitude WHERE node_ID = :node_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':last_transmission', $Node->last_transmission);
    $statement->bindvalue(':longitude', $longitude_new);
    $statement->bindvalue(':node_ID', $node_ID);

    // Execute prepared SQL statement
    $statement->execute();
?>