import React from 'react';

const RouteOptions = ({ routes }) => {
    if (!routes.length) return <div>Hen�z rota hesaplanmad�.</div>;

    return (
        <div>
            <h3>Rota Se�enekleri</h3>
            <table>
                <thead>
                    <tr>
                        <th>Rota T�r�</th>
                        <th>Toplam S�re</th>
                        <th>Toplam �cret</th>
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
