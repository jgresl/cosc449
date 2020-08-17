<?php
    // Prepare SQL statement
    $sql = "UPDATE Gateway SET gateway_description = :gateway_description WHERE gateway_ID = :gateway_ID";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':gateway_description', $description_new);
    $statement->bindvalue(':gateway_ID', $gateway_ID);
    
    // Execute prepared SQL statement
    $statement->execute();
?>