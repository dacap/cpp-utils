# Chrono Class

Minimal class to measure code performance.
This is the public interface of the class:

    class Chrono {
    public:
      Chrono();
      void reset();
      double elapsed() const;
    };

## Example

    {
      Chrono t;
      {
         // Do something...
      }
      std::cout << "Elapsed seconds: " << t.elapsed() << "\n";
    }
