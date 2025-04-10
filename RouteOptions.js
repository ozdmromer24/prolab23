import React from 'react';

const RouteOptions = ({ routes }) => {
    if (!routes.length) return <div>Henüz rota hesaplanmadý.</div>;

    return (
        <div>
            <h3>Rota Seçenekleri</h3>
            <table>
                <thead>
                    <tr>
                        <th>Rota Türü</th>
                        <th>Toplam Süre</th>
                        <th>Toplam Ücret</th>
                        <th>Mesafe</th>
                    </tr>
                </thead>
                <tbody>
                    {routes.map((route, index) => (
                        <tr key={index}>
                            <td>{route.type}</td>
                            <td>{route.sure.toFixed(2)} dk</td>
                            <td>{route.ucret.toFixed(2)} TL</td>
                            <td>{route.mesafe.toFixed(2)} km</td>
                        </tr>
                    ))}
                </tbody>
            </table>
        </div>
    );
};

export default RouteOptions;
