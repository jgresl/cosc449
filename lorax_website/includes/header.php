<div id="masthead">
<a href="main.php"><img src="images/scratch.png" alt="Scratch Post" id="logo"></a>
   <form id="search" method="get" action="./main.php">
      <input type="text" id="searchField" name="search" placeholder="What are you looking for?">
      <button type="submit" id="searchButton"><img src="images/search.png" style="height: 1em"></button>
   </form>

    <?php
        session_start();
        if (isset($_SESSION['username'])) {                    
            echo '<div class="dropdown">';
            echo '<button class="dropbtn">' . $_SESSION['username'] . '</button>';
            echo '<div class="dropdown-content">';
            echo '<a href="profile.php">My Profile</a>';
            if(isset($_SESSION['user_type']) && strcmp($_SESSION['user_type'],'Admin') == 0) echo '<a href="admin.php?command=users">Admin Portal</a>';
            echo '<a href="includes/db_logout.php">Sign Out</a>';
            echo '</div>';
            echo '</div>';
        } else {
            echo '<a href="login.php" class="navButton" id="loginButton">LOG IN</a>';
            echo '<a href="register.php" class="navButton" id="signupButton">SIGN UP</a>';
        }
    ?>

</div>