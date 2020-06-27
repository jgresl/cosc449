function initMap() {
  var gateway = { lat: 49.859184, lng: -119.605215 };
  var nodes = [
    ['1', 49.859150, -119.605215],
    ['2', 49.859160, -119.605225],
    ['3', 49.859170, -119.605235],
    ['4', 49.859180, -119.605245],
    ['5', 49.859190, -119.605255]
  ];
 
  var map = new google.maps.Map(document.getElementById('map'), {
    zoom: 20,
    center: gateway,
    mapTypeId: 'satellite',
  });

  var gateway_marker = new google.maps.Marker({
    position: gateway,
    label: 'G',
    map: map,
  });

  // Generate markers for each node
  for (i = 0; i < nodes.length; i++) {
    marker = new google.maps.Marker({
      position: new google.maps.LatLng(nodes[i][1], nodes[i][2]),
      label: nodes[i][0],
      map: map
    });
  }

  // Create the initial InfoWindow.
  var infoWindow = new google.maps.InfoWindow(
    { content: 'Click the map to get Lat/Lng!', position: gateway });
  infoWindow.open(map);

  // Configure the click listener.
  map.addListener('click', function (mapsMouseEvent) {
    // Close the current InfoWindow.
    infoWindow.close();

    // Create a new InfoWindow.
    infoWindow = new google.maps.InfoWindow({ position: mapsMouseEvent.latLng });
    infoWindow.setContent(mapsMouseEvent.latLng.toString());
    infoWindow.open(map);
  });
}