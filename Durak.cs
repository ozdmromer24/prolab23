namespace IzmitUlasimAPI.Models;

public class Durak
{
    public string Id { get; set; }
    public string Name { get; set; }
    public Konum Konum { get; set; }
    public List<NextStop> NextStops { get; set; }
}
