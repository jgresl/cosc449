window.onload = function () {
    var editSelect = document.querySelectorAll(".edit_field");

    for (var i = 0; i < editSelect.length; i++) {
        editSelect[i].addEventListener("change",
            function (e) {
                this.form.submit();
            }
        );
    }
}