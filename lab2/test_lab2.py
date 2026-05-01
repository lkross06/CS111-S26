import pathlib
import subprocess
import unittest
import tempfile
import os

class TestLab2(unittest.TestCase):
    @classmethod
    def _make(cls):
        return subprocess.run(["make"], capture_output=True, text=True)

    @classmethod
    def _make_clean(cls):
        return subprocess.run(["make", "clean"], capture_output=True, text=True)

    @classmethod
    def setUpClass(cls):
        cls.make_result = cls._make()
        cls.make_success = cls.make_result.returncode == 0

    @classmethod
    def tearDownClass(cls):
        cls._make_clean()

    def run_rr(self, content, quantum):
        self.assertTrue(self.make_success, msg=f"make failed: {self.make_result.stderr}")
        with tempfile.NamedTemporaryFile(delete=False) as f:
            f.write(content.encode())
            f.flush()
            temp_name = f.name

        try:
            cl_result = subprocess.check_output(("./rr", temp_name, str(quantum))).decode()
            lines = cl_result.strip().split("\n")
            # We take the last two lines in case there is still debug output
            wait_line = [l for l in lines if "waiting time" in l.lower()][0]
            resp_line = [l for l in lines if "response time" in l.lower()][0]
            
            testAvgWaitTime = float(wait_line.split(":")[1])
            testAvgRespTime = float(resp_line.split(":")[1])
            return testAvgWaitTime, testAvgRespTime
        finally:
            if os.path.exists(temp_name):
                os.remove(temp_name)

    def test_01_original_averages(self):
        """Original provided processes.txt test cases."""
        content = "4\n1, 0, 7\n2, 2, 4\n3, 4, 1\n4, 5, 4\n"
        correct_wait = {1: 5.50, 2: 5.00, 3: 7.00, 4: 4.50, 5: 5.50, 6: 6.25}
        correct_resp = {1: 0.75, 2: 1.50, 3: 2.75, 4: 3.25, 5: 3.25, 6: 4.00}
        for q in range(1, 7):
            w, r = self.run_rr(content, q)
            self.assertEqual(w, correct_wait[q], f"Wait time mismatch at Q={q}")
            self.assertEqual(r, correct_resp[q], f"Resp time mismatch at Q={q}")

    def test_02_original_requeue_tie(self):
        """Original test for arrival and requeue at the same time."""
        content = "4\n1, 0, 7\n2, 3, 4\n3, 4, 1\n4, 6, 4\n"
        correct_wait = {1: 5.00, 2: 5.25, 3: 6.50, 4: 4.00}
        correct_resp = {1: 0.75, 2: 1.50, 3: 2.25, 4: 2.75}
        for q in range(1, 5):
            w, r = self.run_rr(content, q)
            self.assertEqual(w, correct_wait[q], f"Wait time mismatch at Q={q}")
            self.assertEqual(r, correct_resp[q], f"Resp time mismatch at Q={q}")

    def test_03_single_process(self):
        """A single process should always have 0 wait and 0 response time."""
        content = "1\n1, 0, 10\n"
        w, r = self.run_rr(content, 3)
        self.assertEqual(w, 0.00)
        self.assertEqual(r, 0.00)

    def test_04_all_simultaneous(self):
        """Processes arriving at the same time T=0."""
        content = "3\n1, 0, 2\n2, 0, 2\n3, 0, 2\n"
        # Q=1 -> 1,2,3,1,2,3. P1 finishes at 4, P2 at 5, P3 at 6.
        # P1: W=2, R=0 | P2: W=3, R=1 | P3: W=4, R=2
        w, r = self.run_rr(content, 1)
        self.assertEqual(w, 3.00)
        self.assertEqual(r, 1.00)

    def test_05_cpu_idle_gap(self):
        """CPU goes idle between process arrivals."""
        content = "2\n1, 0, 2\n2, 10, 2\n"
        w, r = self.run_rr(content, 5)
        self.assertEqual(w, 0.00)
        self.assertEqual(r, 0.00)

    def test_06_large_quantum_fcfs(self):
        """If Quantum > Max Burst, it should behave like FCFS."""
        content = "3\n1, 0, 5\n2, 1, 5\n3, 2, 5\n"
        # P1: 0-5, P2: 5-10, P3: 10-15
        # P1: W0, R0 | P2: W4, R4 | P3: W8, R8
        w, r = self.run_rr(content, 100)
        self.assertEqual(w, 4.00)
        self.assertEqual(r, 4.00)

    def test_07_short_bursts(self):
        """Processes that finish before the quantum expires."""
        content = "2\n1, 0, 2\n2, 1, 2\n"
        # Q=5. P1 runs 0-2. P2 starts at 2, finishes at 4.
        # P1: W0, R0 | P2: W1, R1 (starts at 2, arrived 1)
        w, r = self.run_rr(content, 5)
        self.assertEqual(w, 0.50)
        self.assertEqual(r, 0.50)

    def test_08_quantum_one_alternating(self):
        """Strict alternating with Q=1."""
        content = "2\n1, 0, 3\n2, 0, 3\n"
        # 1,2,1,2,1,2. P1 finishes at 5, P2 at 6.
        # P1: W2, R0 | P2: W3, R1
        w, r = self.run_rr(content, 1)
        self.assertEqual(w, 2.50)
        self.assertEqual(r, 0.50)

    def test_09_multiple_arrivals_at_preemption(self):
        """Multiple processes arrive exactly when P1 is preempted."""
        content = "3\n1, 0, 2\n2, 2, 2\n3, 2, 2\n"
        # Q=2. P1 runs 0-2. At T=2, P2 and P3 arrive.
        # Queue should be [P2, P3] since P1 finished.
        w, r = self.run_rr(content, 2)
        # P1: W0, R0 | P2: W0, R0 | P3: W2, R2
        self.assertEqual(w, 0.67)
        self.assertEqual(r, 0.67)

    def test_10_long_rotation(self):
        """Test a longer sequence of processes."""
        content = "5\n1,0,2\n2,0,2\n3,0,2\n4,0,2\n5,0,2\n"
        # Q=2. Behaves like FCFS since Burst == Q.
        # 0, 2, 4, 6, 8 -> Wait: (0+2+4+6+8)/5 = 4.00
        w, r = self.run_rr(content, 2)
        self.assertEqual(w, 4.00)
        self.assertEqual(r, 4.00)

    def test_11_varying_arrivals(self):
        """Arrivals spaced out but overlapping."""
        content = "3\n1, 0, 4\n2, 2, 2\n3, 4, 2\n"
        # Q=2.
        # 0-2: P1. (P2 arrives at 2) Queue: [P2, P1]
        # 2-4: P2. (P3 arrives at 4) Queue: [P1, P3] (P2 finishes)
        # 4-6: P1. Queue: [P3] (P1 finishes)
        # 6-8: P3.
        # P1: Fin 6, W2, R0 | P2: Fin 4, W0, R0 | P3: Fin 8, W2, R2
        w, r = self.run_rr(content, 2)
        self.assertEqual(w, 1.33)
        self.assertEqual(r, 0.67)

    def test_12_late_arrival(self):
        """Process arrives much later."""
        content = "2\n1, 0, 2\n2, 100, 2\n"
        w, r = self.run_rr(content, 1)
        self.assertEqual(w, 0.00)
        self.assertEqual(r, 0.00)

if __name__ == "__main__":
    unittest.main()