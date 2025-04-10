namespace IzmitUlasimAPI.Models;

public class Konum
{
    public double Latitude { get; set; }
    public double Longitude { get; set; }

    public double MesafeHesapla(Konum hedef)
    {
        var R = 6371; // Dünya yarıçapı (km)
        var lat1 = Latitude * Math.PI / 180;
        var lat2 = hedef.Latitude * Math.PI / 180;
        var deltaLat = (hedef.Latitude - Latitude) * Math.PI / 180;
        var deltaLon = (hedef.Longitude - Longitude) * Math.PI / 180;

        var a = Math.Sin(deltaLat / 2) * Math.Sin(deltaLat / 2) +
                Math.Cos(lat1) * Math.Cos(lat2) *
                Math.Sin(deltaLon / 2) * Math.Sin(deltaLon / 2);
        var c = 2 * Math.Atan2(Math.Sqrt(a), Math.Sqrt(1 - a));

        return R * c;
    }
}
