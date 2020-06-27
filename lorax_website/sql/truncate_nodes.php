<?php
    // Prepare SQL statement
    $sql = "SET FOREIGN_KEY_CHECKS=0; TRUNCATE TABLE Node; SET FOREIGN_KEY_CHECKS=1;";
    echo $sql;
    $statement = $pdo->prepare($sql);

    // Execute prepared SQL statement
    $statement->execute();
?>