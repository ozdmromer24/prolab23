import React, { useState, useEffect } from 'react';
import { MapContainer, TileLayer, Marker, Popup, Polyline, useMapEvents } from 'react-leaflet';
import L from 'leaflet';

const durakIcon = new L.Icon({
    iconUrl: require('leaflet/dist/images/marker-icon.png'),
    iconRetinaUrl: require('leaflet/dist/images/marker-icon-2x.png'),
    shadowUrl: require('leaflet/dist/images/marker-shadow.png'),
    iconSize: [25, 41],
    iconAnchor: [12, 41],
    popupAnchor: [1, -34],
    shadowSize: [41, 41]
});

const center = [40.765, 29.940];

const LocationSelector = ({ onLocationSelect }) => {
    const [markerPosition, setMarkerPosition] = useState(null);

    useMapEvents({
        click(e) {
            setMarkerPosition(e.latlng);
            onLocationSelect(e.latlng);
        }
    });

    return markerPosition ? (
        <Marker position={markerPosition}>
            <Popup>Seçilen Konum</Popup>
        </Marker>
    ) : null;
};

const MapComponent = ({ duraklar, onLocationSelect, rota }) => {
    const [polylinePoints, setPolylinePoints] = useState([]);

    useEffect(() => {
        if (rota && rota.length > 0) {
            setPolylinePoints(rota);
        } else {
            setPolylinePoints([]);
        }
    }, [rota]);

    return (
        <MapContainer center={center} zoom={13} style={{ height: "400px", width: "100%" }}>
            <TileLayer
                attribution='&copy; OpenStreetMap'
                url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
            />
            const polylinePoints = rota?.map(durak => [durak.konum.latitude, durak.konum.longitude]) || [];

            {polylinePoints.length > 1 && <Polyline positions={polylinePoints} color="blue" />}
            {/* Durak Marker'larý */}
            {duraklar.map((durak, index) => (
                <Marker
                    key={index}
                    position={[durak.konum.latitude, durak.konum.longitude]}
                    icon={durakIcon}
                >
                    <Popup>{durak.name}</Popup>
                </Marker>
            ))}

            {/* Kullanýcý tarafýndan seçilen konum */}
            <LocationSelector onLocationSelect={onLocationSelect} />

            {/* Rota çizimi */}
            {polylinePoints.length > 1 && (
                <Polyline positions={polylinePoints} color="blue" />
            )}
        </MapContainer>
    );
};

export default MapComponent;
