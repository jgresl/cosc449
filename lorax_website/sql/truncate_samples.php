<?php
    // Prepare SQL statement
    $sql = "TRUNCATE TABLE Sample";
    $statement = $pdo->prepare($sql);

    // Execute prepared SQL statement
    $statement->execute();
?>