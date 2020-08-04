<?php
    // Prepare SQL statement
    $sql = "UPDATE Node SET last_transmission = :last_transmission, sample_frequency = :sample, sync_status = 0 WHERE node_ID = :node_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':last_transmission', $Node->last_transmission);
    $statement->bindvalue(':sample', $sample_new);
    $statement->bindvalue(':node_ID', $node_ID);

    // Execute prepared SQL statement
    $statement->execute();
?>