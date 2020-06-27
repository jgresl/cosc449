<?php

 function openConnection(){
    $connString = "mysql:host=localhost; dbname=lorax";
    $user = "root";
    $pass = "admin";
    $pdo = new PDO($connString, $user, $pass);
    return $pdo;
}

function closeConnection($pdo){
    $pdo = null;
}